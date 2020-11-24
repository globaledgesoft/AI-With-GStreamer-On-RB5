all :
	gcc src/gst_ai_pipeline.c -o gst_ai `pkg-config --cflags --libs gstreamer-1.0`
