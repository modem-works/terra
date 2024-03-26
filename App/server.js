const express = require("express");
const cors = require("cors");
const bodyParser = require("body-parser");
let fetch;

(async () => {
  fetch = (await import("node-fetch")).default;
})();

const app = express();
const port = 3000;

// Middleware
app.use(cors()); // Enable CORS for all routes
app.use(bodyParser.json());

// Replace 'YOUR_OPENAI_API_KEY' with your OpenAI API key
const OPENAI_API_KEY = "YOUR_OPENAI_API_KEY";

app.post("/generate-trail", async (req, res) => {
  const { prompt } = req.body;

  try {
    const response = await fetch("https://api.openai.com/v1/chat/completions", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        Authorization: `Bearer ${OPENAI_API_KEY}`,
      },
      body: JSON.stringify({
        model: "gpt-4", // Make sure to use a model that supports chat
        messages: [
          {
            role: "system",
            content: "You are a helpful assistant.",
          },
          {
            role: "user",
            content: prompt,
          },
        ],
      }),
    });

    if (!response.ok) {
      const errorDetail = await response.json(); // Parsing JSON to get the detailed error message
      console.error("OpenAI response error:", errorDetail);
      throw new Error(`Error from OpenAI API: ${response.statusText}`);
    }

    const data = await response.json();
    console.log("OpenAI API Response:", JSON.stringify(data, null, 2));
    res.json(data);
  } catch (error) {
    console.error(error);
    res.status(500).send("Internal Server Error");
  }
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
