const gst = require("./build/Release/gst_binding");
const sharp = require("sharp");
const fs = require("fs");
const path = require("path");

// Create output directory if it doesn't exist
const outputDir = path.join(__dirname, "frames");
if (!fs.existsSync(outputDir)) {
  fs.mkdirSync(outputDir);
}

// Create a new GStreamer pipeline instance
const pipeline = new gst.GstNodePipeline();

// Set up RTSP pipeline with appsink
const pipelineString = `rtspsrc location=rtsp://192.168.0.215:554/main latency=0 ! 
    queue ! 
    rtph264depay ! 
    h264parse ! 
    avdec_h264 ! 
    videoconvert ! 
    video/x-raw,format=RGB ! 
    appsink name=sink emit-signals=true sync=false max-buffers=1 drop=true`;

let frameCount = 0;
const maxFrames = 10; // Save only 10 frames for testing

// Set up frame callback
pipeline.setFrameCallback((frameBuffer) => {
  // frameBuffer is a Buffer containing the raw RGB frame data
  console.log(`Received frame of size: ${frameBuffer.length} bytes`);

  if (frameCount < maxFrames) {
    // Convert raw RGB buffer to JPEG using sharp
    sharp(frameBuffer, {
      raw: {
        width: 640, // Adjust based on your camera's resolution
        height: 480, // Adjust based on your camera's resolution
        channels: 3,
      },
    })
      .jpeg()
      .toFile(path.join(outputDir, `frame_${frameCount}.jpg`))
      .then(() => {
        console.log(`Saved frame ${frameCount} as JPEG`);
        frameCount++;

        // Stop after saving maxFrames
        if (frameCount >= maxFrames) {
          console.log("Reached maximum frame count, stopping...");
          pipeline.stop();
          process.exit(0);
        }
      })
      .catch((err) => {
        console.error("Error saving frame:", err);
      });
  }
});

// Set the pipeline
pipeline.setPipeline(pipelineString);

// Start the pipeline
pipeline.start();

// Handle cleanup on exit
process.on("SIGINT", () => {
  pipeline.stop();
  process.exit();
});
