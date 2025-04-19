# SecYAN Multiparty PSI Web Frontend

This is a web frontend for the SecYAN Multiparty PSI application. It allows you to run the multiparty PSI protocol through a user-friendly web interface.

## Features

- Start both server and client processes with a single click
- Select query type, data size, result protection, and dual execution options
- Execute queries and view results from both server and client
- Monitor connection status and process output

## Setup

The web frontend is built and installed automatically as part of the main build process. The files are copied to the `build/src/example/web` directory, and dependencies are installed during the build.

## Usage

1. Build the project as normal:
   ```
   mkdir build
   cd build
   cmake ..
   make
   ```

2. Navigate to the web directory:
   ```
   cd src/example/web
   ```

3. Start the web server:
   ```
   npm start
   ```

4. Open a web browser and go to:
   ```
   http://localhost:3000
   ```

5. Use the web interface to:
   - Connect server and client processes
   - Select query parameters
   - Execute queries
   - View results

## How It Works

The web frontend consists of:
- A Node.js Express server (`server.js`) that spawns and manages the server and client processes
- An HTML/CSS frontend (`index.html`) for user interaction
- JavaScript code (`frontend.js`) that handles the UI logic and API calls

The server manages the lifecycle of the `mpsi` executable, automatically feeding it the appropriate inputs based on user selections in the web UI.

## Requirements

- Node.js (v12 or higher)
- npm
- Modern web browser

## Troubleshooting

- If the connection fails, check that the `mpsi` executable is properly built
- If results don't appear, check the server and client logs in the web UI for error messages
- Make sure port 3000 is available for the web server
- Verify that the path to the `mpsi` executable in `server.js` is correct for your build 