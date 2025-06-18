{
  "targets": [{
    "target_name": "gst_binding",
    "cflags!": [ "-fno-exceptions" ],
    "cflags_cc!": [ "-fno-exceptions" ],
    "sources": [ "src/gst_binding.cpp" ],
    "include_dirs": [
      "<!@(node -p \"require('node-addon-api').include\")",
      "D:/gstreamer/1.0/msvc_x86_64/include",
      "D:/gstreamer/1.0/msvc_x86_64/include/gstreamer-1.0",
      "D:/gstreamer/1.0/msvc_x86_64/include/glib-2.0",
      "D:/gstreamer/1.0/msvc_x86_64/lib/glib-2.0/include"
    ],
    "libraries": [
      "D:/gstreamer/1.0/msvc_x86_64/lib/gstreamer-1.0.lib",
      "D:/gstreamer/1.0/msvc_x86_64/lib/gstapp-1.0.lib",
      "D:/gstreamer/1.0/msvc_x86_64/lib/gstvideo-1.0.lib",
      "D:/gstreamer/1.0/msvc_x86_64/lib/glib-2.0.lib",
      "D:/gstreamer/1.0/msvc_x86_64/lib/gobject-2.0.lib"
    ],
    "defines": [ 
      "NAPI_DISABLE_CPP_EXCEPTIONS",
      "GSTREAMER_STATIC"
    ],
    "msvs_settings": {
      "VCCLCompilerTool": {
        "ExceptionHandling": 1,
        "AdditionalIncludeDirectories": [
          "<!(node -p \"require('node-addon-api').include\")",
          "D:/gstreamer/1.0/msvc_x86_64/include",
          "D:/gstreamer/1.0/msvc_x86_64/include/gstreamer-1.0",
          "D:/gstreamer/1.0/msvc_x86_64/include/glib-2.0",
          "D:/gstreamer/1.0/msvc_x86_64/lib/glib-2.0/include"
        ],
        "AdditionalOptions": ["/EHsc"]
      },
      "VCLinkerTool": {
        "AdditionalLibraryDirectories": [
          "D:/gstreamer/1.0/msvc_x86_64/lib"
        ]
      }
    }
  }]
} 