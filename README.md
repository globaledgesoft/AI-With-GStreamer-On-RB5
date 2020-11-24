# Ai with Gstreamer on RB5 - Performance Analysis
## Introduction 
Qualcomm provides its own version of GStreamer which allows you  to load and execute AI  models. It currently has support for SNPE and TF Lite .Elements for GStreamer for executing AI models  are  qtimlesnpe for SNPE and qtimletflite for TF Lite.
qtimletflite element exposes the Tensorflow Lite capabilities to gstreamer. qtimlesnpe element exposes qualcomm neural processing engine capabilities to gstreamer . These two plugins are created to perform preprocessing and postprocessing. Preprocessing supports color convert, Downscale and mean subtraction and padding. Post Processing supports most popular model types as detection, classification and segmentation. 

For more reference visit link given below :
https://developer.qualcomm.com/qualcomm-robotics-rb5-kit/software-reference-manual/application-semantics/gstreamer-plugins


## Prerequisites
 - flash latest image of RB5 as instructed in Thundercomm documentations
 - setting up the SNPE sdk on host system for model conversion
 - Install the Tensorflow on the host system
 - Pushing the binary of SNPE tools for aarch-linux-gcc 4.9 


## Model Conversion
### Converting Tensorflow model in TF Lite 
follow the steps given in url mention below for converting Tensorflow models to the TF Lite :
https://www.tensorflow.org/api_docs/python/tf/compat/v1/lite/TFLiteConverter


### Converting Tensorflow model to the SNPE DLC
make sure that SNPE sdk has been already set up on the host system. Use snpe-tensorflow-to-dlc tool for converting tensorflow models to the SNPE. 
Follow the instruction given for converting Tensorflow model to the SNPE :
https://developer.qualcomm.com/docs/snpe/tools.html#tools_snpe-tensorflow-to-dlc


## Performance tested on 


<table><tr><th>Models</th>
<th>Model link</th>
<th>Model size</th>
<th>Input size</th></tr>
<tr>
<td>Inception_v3</td>
<td>https://storage.googleapis.com/download.tensorflow.org/models/inception_v3_2016_08_28_frozen.pb.tar.gz</td>
<td>92MB</td>
<td>1,299,299,3</td>
</tr>
<tr>
<td>Mobilenet_v1</td>
<td>http://download.tensorflow.org/models/mobilenet_v1_2018_02_22/mobilenet_v1_1.0_224.tgz</td>
<td>17MB</td>
<td>1,224,224,3</td>
</tr>
<tr>
<td>Mobilenet_v2</td>
<td>https://meet.google.com/linkredirect?authuser=0&dest=http%3A%2F%2Fdownload.tensorflow.org%2Fmodels%2Fmobilenet_v1_2018_02_22%2Fmobilenet_v1_1.0_224.tgz</td>
<td>14MB</td>
<td>1,224,224,3</td>
</tr>
<tr>
<td>MobilenetSSD_v1</td>
<td>https://storage.googleapis.com/download.tensorflow.org/models/tflite/coco_ssd_mobilenet_v1_1.0_quant_2018_06_29.zip </td>
<td>28MB</td>
<td>1,300,300,3</td>
</tr>
</table>


## Running the application on RB5

### Building camera and AI pipeline in GStreamer
 1. Clone the gitlab repository using command given below:
```sh
user@user:~/ $ git clone <URL OF THIS PROJECT>

```

 2. Building the binary for gstreamer camera and AI pipeline source, run below command from the project directory:

 ```sh
user@user:~/ $ make
 ```

 3. To run the source code enter the following commands

 ```sh
 user@user:~/ $ ./gst_ai  TFLITE/SNPE /path/to/model.tflite    
```


### Steps to load and execute AI model in RB5 board using gstreamer plugins 
 1. Create directory named  rb5 in the home directory of RB5 board
```sh
 user@user:~/ $ adb shell
 sh:~$ mkdir /home/rb5
 sh:~$ exit
```

 2. Push the AI model and labels to the RB5 board from host system
 ```sh
user@user:~/ $ adb push model_name.tflite /home/rb5
user@user:~/ $ adb push labelmap.txt  home/rb5
```

 3. Create configuration file by gstreamer property. File extension should be .config


### Running model with qtimletflite
```sh
user@user:~/ $ gst-launch-1.0 v4l2src ! jpegdec ! videoconvert ! qtimletflite config=/home/rb5/mle_tflite.config model=/home/rb5/mobilenet_v1.tflite labels=/home/rb5/labelmap.txt postprocessing=segmentation ! videoconvert ! jpegenc ! filesink location=image.jpeg
```
Above pipeline takes the inference frames from the camera source and are delivered to the GStreamer TF Lite plugin along with a model tflite. The TF Lite runtime can be running on the DSP, GPU or CPU. Inference results are gathered back in the GStreamer TF sink for postprocessing and that metadata is stored in the file.

### Running model with qtimlesnpe
```sh
user@user:~/ $ gst-launch-1.0 v4l2src ! jpegdec ! videoconvert ! qtimlesnpe config=/home/rb5/mle_snpe.config model=/home/rb5/mobilenet_v1.dlc labels=/home/rb5/labelmap.txt postprocessing=segmentation ! videoconvert ! jpegenc ! filesink location=image.jpeg
```

Above pipeline takes the inference frames from the camera source and are delivered to the GST-SNE sink along with a model DLC. The GStreamer SNPE Sink will in turn use Qualcomm Neural Processing SDK runtime to offload model computation to the requested runtime(DSP, GPU or GPU). Inference results are gathered back in the GST-SNPE sink for post processing and that data is stored in the file.

`Note : SNPE models did not work on the device as qtimlesnpe element is not available in GStreamer installed.So because of this issue, we had tested models using snpe-throughput-net-run`


`NOTE: To change delegate, open the config file change the delegate        parameter, It can be set to CPU/GPU/DSP.`

## Performance of Ai Models using Gstreamer TFlite
<table><tr>
<th>Models</th>
<th>Model size</th>
<th>DSP (fps)</th>
<th>CPU(fps)</th>
<th>GPU(fps)</th>
</tr>
<tr>
<td>MobilenetSSD_v1</td>
<td>4MB</td>
<td>14 - 15</td>
<td>13 - 14</td>
<td>13 - 14</td>
</tr>
<tr>
<td> Inception_v3</td>
<td>23MB</td>
<td>10 - 11</td>
<td>10 - 11</td>
<td>10 - 11</td>
</tr>
<tr>
<td> Mobilenet_v1</td>
<td> 17MB</td>
<td>14 - 15</td>
<td>14 - 15</td>
<td>14 - 15</td>
</tr>
<tr>
<td> Mobilenet_v2</td>
<td>14 MB</td>
<td>14 - 15</td>
<td>13 - 14</td>
<td>13 - 14</td>
</tr>
<table>


### Measure the inference throughput by snpe-throughput-net-run 
snpe-throughput-net-run runs multiple instances of SNPE for a certain duration of time and measures inference throughput. Each instance of SNPE can have its own model, designated runtime and performance profile.

```sh
snpe-throughput-net-run  --container /home/rb5/mobilenet_v1.dlc  --duration 30 --use_cpu --perf_profile “high_performance” --input_raw /home/rb5/224_mob.raw
```

### Performance Using SNPE on CPU

<table>
<tr>
<th>Models</th>
<th>SNPE model size</th>
<th>Total throughput(infs/sec)</th>
</tr>
<tr>
<td>Inception_v3</td>
<td>    92MB</td>
<td>1.20</td>
</tr>
<tr>
<td>Mobilenet_v1</td>
<td> 14MB</td>
<td>5.77</td>
</tr>
<tr>
<td>Mobilenet_v2</td>
<td>14MB</td>
<td>5.75</td>
</tr>
<tr>
<td>MobilenetSSD_v1</td>
<td>27MB</td>
<td>3.31</td>
</tr>
</table>

`Note : DSP and GPU runtime for SNPE is not available on RB5 platform. So that     unable to create an SNPE instance.`