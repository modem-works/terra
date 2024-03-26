let selectedCoordinates = ""; // Global or higher scope variable to store coordinates

function initAutocomplete() {
  var input = document.getElementById("autocomplete");
  var autocomplete = new google.maps.places.Autocomplete(input);

  autocomplete.addListener("place_changed", function () {
    var place = autocomplete.getPlace();
    if (place.geometry) {
      // Extract the latitude and longitude from the selected place
      var lat = place.geometry.location.lat();
      var lon = place.geometry.location.lng();
      selectedCoordinates = `${lat},${lon}`; // Store the coordinates in a format suitable for your prompt
    }
  });
}

document.querySelectorAll(".custom-option").forEach((option) => {
  option.addEventListener("click", function () {
    if (!this.classList.contains("selected")) {
      this.parentNode
        .querySelector(".custom-option.selected")
        .classList.remove("selected");
      this.classList.add("selected");
      this.closest(".custom-select").querySelector(
        ".custom-select__trigger span"
      ).textContent = this.textContent;
    }
  });
});

document.addEventListener("DOMContentLoaded", function () {
  const form = document.getElementById("address-form");
  const warningMessageDiv = document.getElementById("warningMessage");

  form.addEventListener("submit", async function (event) {
    event.preventDefault(); // Prevent the default form submission behavior

    const addressInput = document.getElementById("autocomplete");
    const timeSelection = document.getElementById("time-selection");

    const address = addressInput.value.trim();
    const time = timeSelection.value;

    if (!address || time === "default") {
      warningMessageDiv.textContent =
        "Enter address and duration before submitting";
      warningMessageDiv.style.visibility = "visible";
      warningMessageDiv.style.opacity = "1";
      return; // Stop further execution if validation fails
    } else {
      warningMessageDiv.style.visibility = "hidden";
      warningMessageDiv.style.opacity = "0";
    }

    // Hide form elements
    addressInput.style.display = "none";
    timeSelection.style.display = "none";
    document.querySelector(".btn").style.display = "none";
    document.getElementById("routeMessage").style.display = "block";

    // Your existing code to hide form elements and display route message...
    document.getElementById("autocomplete").style.display = "none";
    document.getElementById("time-selection").style.display = "none";
    document.querySelector(".btn").style.display = "none";
    document.getElementById("routeMessage").style.display = "block";

    // Determine the number of stops based on the selected time
    let stops;
    switch (time) {
      case "5":
        stops = 5;
        break;
      case "10":
        stops = 6;
        break;
      case "30":
        stops = 8;
        break;
      case "60":
        stops = 10;
        break;
      default:
        stops = 5; // Default to 5 stops if none of the specified times match
    }

    const prompt = `Generate a ${time} walking trail starting from ${selectedCoordinates} including ${stops} stops that ends again at the starting point. Format the route by listing only the GPS coordinates, no additional text. Include the GPS coordinate of the start point at the beginning and end of the list. Label them "start point: ", "checkpoint 1: ", "checkpoint 2 ", etc. followed by "end point: ".`;

    try {
      const response = await fetch("http://localhost:3000/generate-trail", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ prompt: prompt }),
      });

      if (!response.ok) {
        throw new Error("Network response was not ok");
      }

      const data = await response.json();
      const formattedResponse = data.choices[0].message.content; // Assuming this is your response structure

      console.log(formattedResponse);

      // Use the parsing function right after getting the response
      const { startLat, startLon, stopLats, stopLons } =
        parseCoordinatesFromResponse(formattedResponse);

      // Append startLat and startLon to the end of stopLats and stopLons arrays respectively
      stopLats.push(startLat);
      stopLons.push(startLon);

      // Now use these variables to construct your configContent
      const formatArray = (arr) => arr.join(", ");
      // Assuming configContent is generated with potential unwanted indentation
      // Directly create configContent without leading spaces, using template literals properly
      const configContent = [
        "// config.h",
        "#ifndef CONFIG_H",
        "#define CONFIG_H",
        "",
        `const double startLat = ${startLat};`,
        `const double startLon = ${startLon};`,
        "",
        `const double stopLats[] = { ${formatArray(stopLats)} };`,
        `const double stopLons[] = { ${formatArray(stopLons)} };`,
        "",
        `const int numberOfStops = ${stopLats.length};`, // Since stopLats now includes the starting point again, no need to adjust the count
        "",
        "#endif",
      ].join("\n"); // Join array elements with newline character to ensure line breaks

      console.log(configContent);

      // Convert the configContent string into a Blob
      const blob = new Blob([configContent], { type: "text/plain" });

      // Generate a URL for the Blob
      const fileUrl = URL.createObjectURL(blob);

      // Create a download button if it doesn't already exist
      let downloadBtn = document.getElementById("download-btn");
      if (!downloadBtn) {
        downloadBtn = document.createElement("a");
        downloadBtn.id = "download-btn";
        downloadBtn.textContent = "DOWNLOAD TRAIL FILE"; // Button text
        downloadBtn.className = "btn"; // Reuse your button styles
        // document.body.appendChild(downloadBtn); // Adjust where you want the button to be appended
        document.getElementById("content").appendChild(downloadBtn);
      }

      // Set attributes for downloading
      downloadBtn.setAttribute("href", fileUrl);
      downloadBtn.setAttribute("download", "config.h"); // Name of the file to be downloaded

      // Hide the generating message and show the download button
      document.getElementById("routeMessage").style.display = "none";
      downloadBtn.style.display = "block";
    } catch (error) {
      console.error("Error:", error);
      // Handle the error...
    }
  });
});

function parseCoordinatesFromResponse(formattedResponse) {
  // Make the regex case-insensitive with the 'i' flag
  const regex =
    /(?:start point|checkpoint \d+|end point):\s*([+-]?\d+\.\d+),\s*([+-]?\d+\.\d+)/gi;
  let match;
  const startLatLon = [];
  const stopLats = [];
  const stopLons = [];

  while ((match = regex.exec(formattedResponse.toLowerCase())) !== null) {
    // Convert to lowercase for matching
    console.log(match); // Debugging line to see matches

    const lat = parseFloat(match[1]);
    const lon = parseFloat(match[2]);

    // Now checking against the lowercase, so the input text is converted to lowercase for comparison
    const label = match[0].toLowerCase();
    if (label.includes("start point") || label.includes("end point")) {
      startLatLon.push({ lat, lon });
    } else {
      stopLats.push(lat);
      stopLons.push(lon);
    }
  }

  console.log("Start/End:", startLatLon); // Check what's captured for start and end points

  if (startLatLon.length === 0) {
    console.error("No start or end point found");
    return null; // or some error handling
  }

  // Ensure that there is at least one startLatLon entry before attempting to destructure
  const { lat: startLat, lon: startLon } = startLatLon[0] || {
    lat: undefined,
    lon: undefined,
  };
  return { startLat, startLon, stopLats, stopLons };
}
