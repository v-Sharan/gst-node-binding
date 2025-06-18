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
      path: require.resolve("path-browserify"),
      fs: false,
      crypto: require.resolve("crypto-browserify"),
      stream: require.resolve("stream-browserify"),
      util: require.resolve("util"),
      buffer: require.resolve("buffer/"),
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
      // Important: Allow native modules
      nativeWindowOpen: true,
    },
  });

  // Load your app
  win.loadFile("index.html");
}

app.whenReady().then(createWindow);
```

### 3. Create Direct VideoStream Component

```jsx
// src/components/VideoStream.jsx
import React, { useEffect, useRef, useState } from "react";
import GStreamerPipeline from "gst-node-bindings";

const VideoStream = ({ rtspUrl, width = 640, height = 480 }) => {
  const videoRef = useRef(null);
  const pipelineRef = useRef(null);
  const [error, setError] = useState(null);
  const [isLoading, setIsLoading] = useState(true);

  useEffect(() => {
    try {
      // Create pipeline instance
      const pipeline = new GStreamerPipeline();
      pipelineRef.current = pipeline;

      // Set up RTSP pipeline with low latency
      const pipelineString = `rtspsrc location=${rtspUrl} latency=0 ! 
        queue max-size-buffers=1 ! 
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
          setIsLoading(false);
        }
      });

      // Set and start the pipeline
      pipeline.setPipeline(pipelineString);
      pipeline.start();
    } catch (err) {
      console.error("Error initializing pipeline:", err);
      setError(err.message);
    }

    // Cleanup on unmount
    return () => {
      if (pipelineRef.current) {
        try {
          pipelineRef.current.stop();
        } catch (err) {
          console.error("Error stopping pipeline:", err);
        }
      }
    };
  }, [rtspUrl]);

  if (error) {
    return (
      <div
        style={{
          width,
          height,
          border: "1px solid #ccc",
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
        }}
      >
        <p>Error: {error}</p>
      </div>
    );
  }

  return (
    <div style={{ width, height, border: "1px solid #ccc" }}>
      {isLoading ? (
        <div
          style={{
            width: "100%",
            height: "100%",
            display: "flex",
            alignItems: "center",
            justifyContent: "center",
          }}
        >
          <p>Loading stream...</p>
        </div>
      ) : (
        <img
          ref={videoRef}
          style={{ width: "100%", height: "100%", objectFit: "contain" }}
          alt="RTSP Stream"
        />
      )}
    </div>
  );
};

export default VideoStream;
```

### 4. Use in Your App

```jsx
// src/App.jsx
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

### 5. Environment Setup

1. **Install Dependencies**:

```bash
npm install --save-dev electron electron-builder
```

2. **Add Scripts to package.json**:

```json
{
  "scripts": {
    "start": "electron .",
    "build": "electron-builder"
  }
}
```

### 6. Error Handling

Add error handling to the VideoStream component:

```jsx
// src/components/VideoStream.jsx
const VideoStream = ({ rtspUrl, width = 640, height = 480 }) => {
  const videoRef = useRef(null);
  const pipelineRef = useRef(null);
  const [error, setError] = useState(null);

  useEffect(() => {
    try {
      const pipeline = new GStreamerPipeline();
      pipelineRef.current = pipeline;

      // ... rest of the setup code ...
    } catch (err) {
      console.error("Error initializing pipeline:", err);
      setError(err.message);
    }

    return () => {
      if (pipelineRef.current) {
        try {
          pipelineRef.current.stop();
        } catch (err) {
          console.error("Error stopping pipeline:", err);
        }
      }
    };
  }, [rtspUrl]);

  if (error) {
    return (
      <div
        style={{
          width,
          height,
          border: "1px solid #ccc",
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
        }}
      >
        <p>Error: {error}</p>
      </div>
    );
  }

  // ... rest of the component ...
};
```

### 7. Performance Optimization

1. **Adjust Frame Rate**:

```jsx
useEffect(() => {
  const pipeline = new GStreamerPipeline();
  pipelineRef.current = pipeline;

  // Set lower latency for better performance
  const pipelineString = `rtspsrc location=${rtspUrl} latency=0 ! 
    queue max-size-buffers=1 ! 
    rtph264depay ! 
    h264parse ! 
    avdec_h264 ! 
    videoconvert ! 
    video/x-raw,format=RGB ! 
    appsink name=sink emit-signals=true sync=false max-buffers=1 drop=true`;

  // ... rest of the code ...
}, [rtspUrl]);
```

2. **Add Loading State**:

```jsx
const [isLoading, setIsLoading] = useState(true);

useEffect(() => {
  const pipeline = new GStreamerPipeline();
  pipelineRef.current = pipeline;

  pipeline.onFrame((dataUrl) => {
    if (videoRef.current) {
      videoRef.current.src = dataUrl;
      setIsLoading(false);
    }
  });

  // ... rest of the code ...
}, [rtspUrl]);

return (
  <div style={{ width, height, border: "1px solid #ccc" }}>
    {isLoading ? (
      <div
        style={{
          width: "100%",
          height: "100%",
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
        }}
      >
        <p>Loading stream...</p>
      </div>
    ) : (
      <img
        ref={videoRef}
        style={{ width: "100%", height: "100%", objectFit: "contain" }}
        alt="RTSP Stream"
      />
    )}
  </div>
);
```

### 8. Build and Run

```bash
# Start the app
npm start

# Build for distribution
npm run build
```

## Direct Usage with Electron

To use GStreamer bindings directly in your Electron app without a server:

### 1. Update Webpack Configuration

```javascript
// webpack.config.js
const path = require("path");
const webpack = require("webpack");

module.exports = {
  // Important: Must be electron-renderer for native modules
  target: "electron-renderer",

  // ... your existing config ...

  resolve: {
    // ... your existing resolve config ...
    fallback: {
      // ... your existing fallbacks ...
      path: require.resolve("path-browserify"),
      fs: false,
      crypto: require.resolve("crypto-browserify"),
      stream: require.resolve("stream-browserify"),
      util: require.resolve("util"),
      buffer: require.resolve("buffer/"),
    },
  },

  module: {
    rules: [
      // ... your existing rules ...
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

### 2. Update Electron Main Process

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
      // Important: Allow native modules
      nativeWindowOpen: true,
    },
  });

  // Load your app
  win.loadFile("index.html");
}

app.whenReady().then(createWindow);
```

### 3. Create Direct VideoStream Component

```jsx
// src/components/VideoStream.jsx
import React, { useEffect, useRef, useState } from "react";
import GStreamerPipeline from "gst-node-bindings";

const VideoStream = ({ rtspUrl, width = 640, height = 480 }) => {
  const videoRef = useRef(null);
  const pipelineRef = useRef(null);
  const [error, setError] = useState(null);
  const [isLoading, setIsLoading] = useState(true);

  useEffect(() => {
    try {
      // Create pipeline instance
      const pipeline = new GStreamerPipeline();
      pipelineRef.current = pipeline;

      // Set up RTSP pipeline with low latency
      const pipelineString = `rtspsrc location=${rtspUrl} latency=0 ! 
        queue max-size-buffers=1 ! 
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
          setIsLoading(false);
        }
      });

      // Set and start the pipeline
      pipeline.setPipeline(pipelineString);
      pipeline.start();
    } catch (err) {
      console.error("Error initializing pipeline:", err);
      setError(err.message);
    }

    // Cleanup on unmount
    return () => {
      if (pipelineRef.current) {
        try {
          pipelineRef.current.stop();
        } catch (err) {
          console.error("Error stopping pipeline:", err);
        }
      }
    };
  }, [rtspUrl]);

  if (error) {
    return (
      <div
        style={{
          width,
          height,
          border: "1px solid #ccc",
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
        }}
      >
        <p>Error: {error}</p>
      </div>
    );
  }

  return (
    <div style={{ width, height, border: "1px solid #ccc" }}>
      {isLoading ? (
        <div
          style={{
            width: "100%",
            height: "100%",
            display: "flex",
            alignItems: "center",
            justifyContent: "center",
          }}
        >
          <p>Loading stream...</p>
        </div>
      ) : (
        <img
          ref={videoRef}
          style={{ width: "100%", height: "100%", objectFit: "contain" }}
          alt="RTSP Stream"
        />
      )}
    </div>
  );
};

export default VideoStream;
```

### 4. Use in Your App

```jsx
// src/App.jsx
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

### 5. Build and Run

```bash
# Start the app
npm start

# Build for distribution
npm run build
```

## API Reference

### GStreamerPipeline

```

```
