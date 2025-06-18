# GStreamer Node.js Bindings

This package provides Node.js bindings for GStreamer, allowing you to access RTSP video streams in your Node.js/Electron applications. It's particularly useful for React applications running in Electron.

## Prerequisites

Before using this package, ensure you have the following installed:

1. **GStreamer**

   - Download and install GStreamer from [gstreamer.freedesktop.org](https://gstreamer.freedesktop.org/download/)
   - Install both Runtime and Development packages
   - Add GStreamer bin directory to your system PATH
     - Windows: `D:\gstreamer\1.0\msvc_x86_64\bin`
     - Linux: `/usr/lib/gstreamer-1.0`

2. **Node.js and npm**

   - Node.js v14 or higher
   - npm v6 or higher

3. **Build Tools**
   - Windows: Visual Studio 2022 with C++ development tools
   - Linux: GCC and build-essential
   - macOS: Xcode Command Line Tools

## Installation

1. Install the package:

```bash
npm install gst-node-bindings
```

2. Install dependencies:

```bash
npm install
```

3. Build the native module:

```bash
npm run install
```

## Installation from GitHub

### 1. Install from GitHub Repository

```bash
# Navigate to your project directory
cd your-project

# Install directly from GitHub
npm install github:your-username/gst-node-bindings

# Or if you've forked the repository
npm install github:your-fork-username/gst-node-bindings
```

### 2. Install Dependencies

```bash
# Install required dependencies
npm install react react-dom

# Install development dependencies
npm install --save-dev webpack webpack-cli webpack-node-externals node-loader
npm install --save-dev @babel/core @babel/preset-env @babel/preset-react babel-loader
```

### 3. Add to Your Project

1. **Update your webpack.config.js**:

```javascript
const nodeExternals = require("webpack-node-externals");

module.exports = {
  target: "electron-renderer", // or "node" if not using Electron
  externals: [nodeExternals()],
  module: {
    rules: [
      {
        test: /\.node$/,
        use: "node-loader",
      },
    ],
  },
};
```

2. **Create a VideoStream component**:

```jsx
// src/components/VideoStream.jsx
import React, { useEffect, useRef } from "react";
import GStreamerPipeline from "gst-node-bindings";

const VideoStream = ({ rtspUrl, width = 640, height = 480 }) => {
  const videoRef = useRef(null);
  const pipelineRef = useRef(null);

  useEffect(() => {
    const pipeline = new GStreamerPipeline();
    pipelineRef.current = pipeline;

    const pipelineString = `rtspsrc location=${rtspUrl} latency=0 ! 
      queue ! 
      rtph264depay ! 
      h264parse ! 
      avdec_h264 ! 
      videoconvert ! 
      video/x-raw,format=RGB ! 
      appsink name=sink emit-signals=true sync=false max-buffers=1 drop=true`;

    pipeline.onFrame((dataUrl) => {
      if (videoRef.current) {
        videoRef.current.src = dataUrl;
      }
    });

    pipeline.setPipeline(pipelineString);
    pipeline.start();

    return () => {
      if (pipelineRef.current) {
        pipelineRef.current.stop();
      }
    };
  }, [rtspUrl]);

  return (
    <div style={{ width, height, border: "1px solid #ccc" }}>
      <img
        ref={videoRef}
        style={{ width: "100%", height: "100%", objectFit: "contain" }}
        alt="RTSP Stream"
      />
    </div>
  );
};

export default VideoStream;
```

3. **Use in your existing component**:

```jsx
// src/App.jsx or your existing component
import React from "react";
import VideoStream from "./components/VideoStream";

function YourComponent() {
  return (
    <div>
      <h1>Your Existing App</h1>
      <VideoStream rtspUrl="rtsp://your-camera-url" width={640} height={480} />
    </div>
  );
}
```

### 4. Environment Setup

1. **Install GStreamer**:

   - Download from [gstreamer.freedesktop.org](https://gstreamer.freedesktop.org/download/)
   - Install both Runtime and Development packages
   - Add to system PATH:

     ```bash
     # Windows
     set PATH=%PATH%;D:\gstreamer\1.0\msvc_x86_64\bin

     # Linux/macOS
     export PATH=$PATH:/usr/lib/gstreamer-1.0
     ```

2. **Build Tools**:
   - Windows: Visual Studio 2022 with C++
   - Linux: GCC and build-essential
   - macOS: Xcode Command Line Tools

### 5. Build and Run

```bash
# Build your project
npm run build

# Start your application
npm start
```

### Troubleshooting

If you encounter issues:

1. **Module not found**:

   ```bash
   Error: Cannot find module 'gst-node-bindings'
   ```

   Solution: Run `npm run install` in the gst-node-bindings directory

2. **Build errors**:

   ```bash
   Error: Can't resolve 'gst-node-bindings'
   ```

   Solution: Check webpack config and ensure `.node` files are handled correctly

3. **GStreamer errors**:
   ```bash
   Error: Cannot find module 'gst/gst.h'
   ```
   Solution: Verify GStreamer installation and PATH settings

## Fixing node-loader Error

If you encounter the error:

```
node-loader: TypeError: process.dlopen is not a function
```

This error occurs because native modules can't run directly in the browser. Here's how to fix it:

### 1. Update Webpack Configuration

```javascript
// webpack.config.js
const path = require("path");
const nodeExternals = require("webpack-node-externals");

module.exports = {
  // Important: Set target to 'electron-renderer' or 'node'
  target: "electron-renderer", // Use 'node' if not using Electron

  // Exclude node modules from the bundle
  externals: [nodeExternals()],

  // Specify the entry point
  entry: "./src/index.js",

  output: {
    path: path.resolve(__dirname, "dist"),
    filename: "bundle.js",
    library: {
      type: "commonjs2",
    },
  },

  resolve: {
    extensions: [".js", ".jsx", ".json"],
    // Add fallbacks for node modules
    fallback: {
      path: false,
      fs: false,
      crypto: false,
    },
  },

  module: {
    rules: [
      {
        test: /\.(js|jsx)$/,
        exclude: /node_modules/,
        use: {
          loader: "babel-loader",
          options: {
            presets: ["@babel/preset-react", "@babel/preset-env"],
          },
        },
      },
      {
        test: /\.node$/,
        use: {
          loader: "node-loader",
          options: {
            name: "[name].[ext]",
          },
        },
      },
    ],
  },
};
```

### 2. Electron Configuration

If you're using Electron, update your main process:

```javascript
// main.js
const { app, BrowserWindow } = require("electron");
const path = require("path");

function createWindow() {
  const win = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false,
      enableRemoteModule: true,
    },
  });

  // Load your app
  win.loadFile("index.html");
}

app.whenReady().then(createWindow);
```

### 3. Alternative Solution: Use a Server

If you can't use Electron, create a server to handle the GStreamer pipeline:

```javascript
// server.js
const express = require("express");
const GStreamerPipeline = require("gst-node-bindings");
const app = express();

// Create pipeline instance
const pipeline = new GStreamerPipeline();

// Set up RTSP pipeline
const pipelineString = `rtspsrc location=rtsp://your-camera-url latency=0 ! 
  queue ! 
  rtph264depay ! 
  h264parse ! 
  avdec_h264 ! 
  videoconvert ! 
  video/x-raw,format=RGB ! 
  appsink name=sink emit-signals=true sync=false max-buffers=1 drop=true`;

// Store latest frame
let latestFrame = null;

pipeline.onFrame((dataUrl) => {
  latestFrame = dataUrl;
});

pipeline.setPipeline(pipelineString);
pipeline.start();

// Serve the latest frame
app.get("/stream", (req, res) => {
  if (latestFrame) {
    res.send(latestFrame);
  } else {
    res.status(404).send("No frame available");
  }
});

app.listen(3000, () => {
  console.log("Server running on port 3000");
});
```

Then in your React component:

```jsx
// VideoStream.jsx
import React, { useEffect, useState } from "react";

const VideoStream = ({ width = 640, height = 480 }) => {
  const [frame, setFrame] = useState(null);

  useEffect(() => {
    const fetchFrame = async () => {
      try {
        const response = await fetch("http://localhost:3000/stream");
        const data = await response.text();
        setFrame(data);
      } catch (error) {
        console.error("Error fetching frame:", error);
      }
    };

    const interval = setInterval(fetchFrame, 100);
    return () => clearInterval(interval);
  }, []);

  return (
    <div style={{ width, height, border: "1px solid #ccc" }}>
      {frame && (
        <img
          src={frame}
          style={{ width: "100%", height: "100%", objectFit: "contain" }}
          alt="RTSP Stream"
        />
      )}
    </div>
  );
};

export default VideoStream;
```

### 4. Environment Variables

Make sure these environment variables are set:

```bash
# Windows
set NODE_ENV=development
set ELECTRON_ENABLE_LOGGING=1

# Linux/macOS
export NODE_ENV=development
export ELECTRON_ENABLE_LOGGING=1
```

### 5. Additional Troubleshooting

If you still encounter issues:

1. **Clear node_modules and rebuild**:

```bash
rm -rf node_modules
npm cache clean --force
npm install
```

2. **Check Node.js version**:

```bash
node -v  # Should be v14 or higher
```

3. **Verify GStreamer installation**:

```bash
# Windows
where gst-launch-1.0

# Linux/macOS
which gst-launch-1.0
```

4. **Enable debug logging**:

```bash
# Windows
set DEBUG=*

# Linux/macOS
export DEBUG=*
```

## API Reference

### GStreamerPipeline

```

```
