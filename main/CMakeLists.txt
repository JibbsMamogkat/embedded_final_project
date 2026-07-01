idf_component_register(SRCS "main.cpp"
                            "roi_cropper.cpp"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp-tflite-micro
                       EMBED_FILES "mobilenet_v2_64x64_quantized.tflite")