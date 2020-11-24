#include <gst/gst.h>
#include <string.h>

#define DELAY_VALUE 1000000

int main(int argc, char *argv[])
{
	GstElement *pipeline, *source, *dec, *convert1, *tflite, *snpe, *convert2, 
			   			*enc, *sink, *fpsink;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;
	
	gchar *fps_msg;
	guint delay_fps = 0;
	
	/* Initialize GStreamer */
	gst_init (&argc, &argv);

	/* Create the empty pipeline */
	pipeline = gst_pipeline_new ("pipeline");

	/* Create the elements */
	source = gst_element_factory_make ("v4l2src", "source");
	dec = gst_element_factory_make ("jpegdec", "dec");
	convert1 = gst_element_factory_make ("videoconvert", "convert1");
	tflite = gst_element_factory_make ("qtimletflite", "tflite");
	snpe = gst_element_factory_make("qtimlesnpe", "snpe");
	convert2 = gst_element_factory_make ("videoconvert", "convert2");
	enc = gst_element_factory_make ("jpegenc", "enc");
	sink = gst_element_factory_make ("filesink", "sink");
	fpsink = gst_element_factory_make ("fpsdisplaysink", "fpsink");

	if (!strcmp("TFLITE", argv[1])) {
		if (!source || !dec || !convert1 || !tflite || !convert2 || !enc 
						|| !fpsink) {
			g_printerr ("Not all elements could be created.\n");
		}
		gst_bin_add_many (GST_BIN (pipeline), source, dec, convert1, tflite, 
						convert2, enc, fpsink, NULL);
		if (!gst_element_link_many (source, dec, convert1, tflite, convert2, 
								enc, fpsink, NULL)) {
			g_printerr ("Elements could not be linked.\n");
		}
		
		g_object_set (source, "device", "/dev/video0", NULL);
		g_object_set (tflite, "config", "./rb5/cfg/mle_tflite.config", "model", 
			argv[2], "labels", "./rb5/cfg/labelmap.txt", "postprocessing", 
			"segmentation", NULL);
		g_object_set (sink, "location", "./rb5/image.jpeg", NULL);	
		g_object_set(fpsink, "text-overlay", FALSE, "video-sink", sink, NULL);
	}
	else if (!strcmp("SNPE", argv[1])) {
		if (!source || !dec || !convert1 || !snpe|| !convert2 || !enc 
						|| !fpsink) {
			g_printerr ("Not all elements could be created.\n");
			return -1;
		}
		gst_bin_add_many (GST_BIN (pipeline), source, dec, convert1, snpe, 
						convert2, enc, fpsink, NULL);
		if (!gst_element_link_many (source, dec, convert1, snpe, convert2, 
								enc, fpsink, NULL)) {
			g_printerr ("Elements could not be linked.\n");
		}
		g_object_set (source, "device", "/dev/video0", NULL);
		g_object_set (snpe, "config", "./rb5/cfg/mle_snpe.config", "model", 
			argv[2], "labels", "./rb5/cfg/labelmap.txt", 
			"postprocessing", "segmentation", NULL);
		g_object_set (sink, "location", "./rb5/image.jpeg", NULL);	
		g_object_set(fpsink, "text-overlay", FALSE, "video-sink", sink, NULL);
	}
	
	/* Set the elements to the playing state */
	g_print("Setting pipeline to Playing State\n");
	ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
	     g_printerr ("Unable to set the pipeline to the playing state.\n");
	}

	bus = gst_element_get_bus (pipeline);
	msg = gst_bus_pop(bus);	

	if (msg != NULL) {
		GError *err;
		gchar *debug_info;

		switch (GST_MESSAGE_TYPE (msg)) {
		    case GST_MESSAGE_ERROR:
			gst_message_parse_error (msg, &err, &debug_info);
			g_printerr ("Error received from element %s : %s\n", 
				GST_OBJECT_NAME (msg->src), err->message);
			g_printerr ("Debugging information : %s\n", debug_info ? 
							debug_info : "none");
			g_clear_error (&err);
			g_free (debug_info);
			break;

		}

		gst_message_unref (msg);
	}

	/* Display fps */
	while (1) {
		g_object_get (fpsink, "last-message", &fps_msg, NULL);
		delay_fps++;
		if (fps_msg != NULL) {
			if ((delay_fps % DELAY_VALUE) == 0) {
				g_print ("Frame information: %s\n", fps_msg);
				delay_fps = 0;
			}
		}
	}

	/* Free resources */
	gst_object_unref (bus);
	
	gst_element_set_state (pipeline, GST_STATE_NULL);
	gst_object_unref (pipeline);

	return 0;	
}
