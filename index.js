const gst = require("./build/Release/gst_binding");

class GStreamerPipeline {
  constructor() {
    this.pipeline = new gst.GstNodePipeline();
    this.frameCallback = null;
    this.frameDataUrl = null;
  }

  setPipeline(pipelineString) {
    this.pipeline.setPipeline(pipelineString);
  }

  start() {
    this.pipeline.start();
  }

  stop() {
    this.pipeline.stop();
  }

  onFrame(callback) {
    this.frameCallback = callback;
    this.pipeline.setFrameCallback((frameBuffer) => {
      // Convert frame buffer to base64 data URL
      const base64 = Buffer.from(frameBuffer).toString("base64");
      this.frameDataUrl = `data:image/jpeg;base64,${base64}`;

      // Call the user's callback with the data URL
      if (callback) {
        callback(this.frameDataUrl);
      }
    });
  }

  getCurrentFrame() {
    return this.frameDataUrl;
  }
}

module.exports = GStreamerPipeline;
