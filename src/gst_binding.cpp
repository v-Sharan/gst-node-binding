#include <napi.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>
#include <string>
#include <vector>

class GstNodePipeline : public Napi::ObjectWrap<GstNodePipeline> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    GstNodePipeline(const Napi::CallbackInfo& info);
    ~GstNodePipeline();

private:
    static Napi::FunctionReference constructor;
    GstElement* pipeline;
    GstElement* appsink;
    Napi::ThreadSafeFunction frameCallback;
    guint bus_watch_id;

    static gboolean bus_call(GstBus* bus, GstMessage* msg, gpointer data);
    static GstFlowReturn new_sample(GstElement* sink, gpointer data);
    Napi::Value SetPipeline(const Napi::CallbackInfo& info);
    Napi::Value Start(const Napi::CallbackInfo& info);
    Napi::Value Stop(const Napi::CallbackInfo& info);
    Napi::Value SetFrameCallback(const Napi::CallbackInfo& info);
};

Napi::FunctionReference GstNodePipeline::constructor;

GstNodePipeline::GstNodePipeline(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GstNodePipeline>(info) {
    pipeline = nullptr;
    appsink = nullptr;
    bus_watch_id = 0;
}

GstNodePipeline::~GstNodePipeline() {
    if (pipeline) {
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }
    if (frameCallback) {
        frameCallback.Release();
    }
}

Napi::Value GstNodePipeline::SetPipeline(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string pipelineStr = info[0].As<Napi::String>().Utf8Value();
    
    if (pipeline) {
        gst_object_unref(pipeline);
    }

    GError* error = nullptr;
    pipeline = gst_parse_launch(pipelineStr.c_str(), &error);
    
    if (error) {
        Napi::Error::New(env, error->message).ThrowAsJavaScriptException();
        g_error_free(error);
        return env.Null();
    }

    // Get the appsink element
    appsink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
    if (!appsink) {
        Napi::Error::New(env, "Could not find appsink element named 'sink'").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Set up the new-sample callback
    g_object_set(G_OBJECT(appsink), "emit-signals", TRUE, NULL);
    g_signal_connect(appsink, "new-sample", G_CALLBACK(new_sample), this);

    // Set up bus watch
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    bus_watch_id = gst_bus_add_watch(bus, bus_call, this);
    gst_object_unref(bus);

    return env.Undefined();
}

GstFlowReturn GstNodePipeline::new_sample(GstElement* sink, gpointer data) {
    GstNodePipeline* self = static_cast<GstNodePipeline*>(data);
    
    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    if (!sample) {
        return GST_FLOW_ERROR;
    }

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    // Create a copy of the frame data
    std::vector<guint8> frameData(map.data, map.data + map.size);

    // Call the JavaScript callback with the frame data
    auto callback = [frameData](Napi::Env env, Napi::Function jsCallback) {
        Napi::Buffer<guint8> buffer = Napi::Buffer<guint8>::Copy(env, frameData.data(), frameData.size());
        jsCallback.Call({buffer});
    };

    napi_status status = self->frameCallback.BlockingCall(callback);
    if (status != napi_ok) {
        g_print("Failed to call JavaScript callback\n");
    }

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

Napi::Value GstNodePipeline::SetFrameCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Function expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Function callback = info[0].As<Napi::Function>();
    frameCallback = Napi::ThreadSafeFunction::New(
        env,
        callback,
        "FrameCallback",
        0,
        1
    );

    return env.Undefined();
}

Napi::Value GstNodePipeline::Start(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!pipeline) {
        Napi::Error::New(env, "Pipeline not set").ThrowAsJavaScriptException();
        return env.Null();
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    return env.Undefined();
}

Napi::Value GstNodePipeline::Stop(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!pipeline) {
        Napi::Error::New(env, "Pipeline not set").ThrowAsJavaScriptException();
        return env.Null();
    }

    gst_element_set_state(pipeline, GST_STATE_NULL);
    return env.Undefined();
}

gboolean GstNodePipeline::bus_call(GstBus* bus, GstMessage* msg, gpointer data) {
    GstNodePipeline* self = static_cast<GstNodePipeline*>(data);
    
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            g_print("End of stream\n");
            gst_element_set_state(self->pipeline, GST_STATE_NULL);
            break;
        case GST_MESSAGE_ERROR: {
            gchar* debug;
            GError* error;
            gst_message_parse_error(msg, &error, &debug);
            g_free(debug);
            g_print("Error: %s\n", error->message);
            g_error_free(error);
            gst_element_set_state(self->pipeline, GST_STATE_NULL);
            break;
        }
        default:
            break;
    }
    return TRUE;
}

Napi::Object GstNodePipeline::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "GstNodePipeline", {
        InstanceMethod("setPipeline", &GstNodePipeline::SetPipeline),
        InstanceMethod("start", &GstNodePipeline::Start),
        InstanceMethod("stop", &GstNodePipeline::Stop),
        InstanceMethod("setFrameCallback", &GstNodePipeline::SetFrameCallback),
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("GstNodePipeline", func);
    return exports;
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    gst_init(nullptr, nullptr);
    return GstNodePipeline::Init(env, exports);
}

NODE_API_MODULE(gst_binding, InitAll) 