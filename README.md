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
