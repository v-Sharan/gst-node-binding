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

## Usage with React and Webpack

### 1. Webpack Configuration

Create or update your `webpack.config.js`:

```javascript
const path = require("path");
const nodeExternals = require("webpack-node-externals");

module.exports = {
  target: "electron-renderer",
  externals: [nodeExternals()],
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
    alias: {
      "@": path.resolve(__dirname, "src"),
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
        use: "node-loader",
      },
    ],
  },
};
```

### 2. React Component

Create a VideoStream component:

```jsx
import React, { useEffect, useRef } from "react";
import GStreamerPipeline from "gst-node-bindings";

const VideoStream = ({ rtspUrl, width = 640, height = 480 }) => {
  const videoRef = useRef(null);
  const pipelineRef = useRef(null);

  useEffect(() => {
    // Create pipeline instance
    const pipeline = new GStreamerPipeline();
    pipelineRef.current = pipeline;

    // Set up RTSP pipeline
    const pipelineString = `rtspsrc location=${rtspUrl} latency=0 ! 
            queue ! 
            rtph264depay ! 
            h264parse ! 
            avdec_h264 ! 
            videoconvert ! 
            video/x-raw,format=RGB ! 
            appsink name=sink emit-signals=true sync=false max-buffers=1 drop=true`;

    // Set up frame callback
    pipeline.onFrame((dataUrl) => {
      if (videoRef.current) {
        videoRef.current.src = dataUrl;
      }
    });

    // Set and start the pipeline
    pipeline.setPipeline(pipelineString);
    pipeline.start();

    // Cleanup on unmount
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

### 3. Using the Component

```jsx
import React from "react";
import VideoStream from "./components/VideoStream";

function App() {
  return (
    <div>
      <h1>RTSP Stream</h1>
      <VideoStream rtspUrl="rtsp://your-camera-url" width={640} height={480} />
    </div>
  );
}

export default App;
```

### 4. Electron Configuration

In your Electron main process:

```javascript
const { app, BrowserWindow } = require("electron");

function createWindow() {
  const win = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false,
    },
  });

  win.loadFile("index.html");
}

app.whenReady().then(createWindow);
```

## API Reference

### GStreamerPipeline

```javascript
const pipeline = new GStreamerPipeline();
```

#### Methods

- `setPipeline(pipelineString: string)`: Set the GStreamer pipeline
- `start()`: Start the pipeline
- `stop()`: Stop the pipeline
- `onFrame(callback: (dataUrl: string) => void)`: Set callback for frame updates
- `getCurrentFrame()`: Get the current frame as a data URL

## Common Issues and Solutions

1. **Missing GStreamer**

   - Error: "Cannot find module 'gst/gst.h'"
   - Solution: Ensure GStreamer is installed and PATH is set correctly

2. **Build Errors**

   - Error: "node-gyp rebuild failed"
   - Solution: Install build tools and ensure they're in PATH

3. **Runtime Errors**
   - Error: "Cannot find module 'gst-node-bindings'"
   - Solution: Rebuild the native module with `npm run install`

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

MIT
