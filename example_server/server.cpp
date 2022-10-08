#include <iostream>
#include "zmq.h"
#include<cassert>
#include <random>
#include<opencv2/opencv.hpp>
#include <unistd.h>
#include "test.capnp.h"
#include<capnp/serialize.h>
using namespace std;

int main()
{
	void *context = zmq_ctx_new();

	
	void* publisher = zmq_socket(context, ZMQ_PUB);
	int rc = zmq_bind(publisher, "tcp://*:5557");
	assert(rc == 0);
	srand((unsigned)time (NULL));
	std::string input;
	cv::VideoCapture capture(0);
 
   if(!capture.isOpened())
   {
      std::cout << "Could not open camera" << std::endl;
      return -1;
   }
   	while(capture.isOpened())
	{
	
		cv::Mat cameraFrame;
		capture.read(cameraFrame);

		// stop if Esc has been pressed:
		if (cv::waitKey(1) == 27) {
			break;
		}

		capnp::MallocMessageBuilder message;
		Image::Builder img = message.initRoot<Image>();
		img.setHeight(cameraFrame.cols);
		img.setWidth(cameraFrame.rows);
		img.setChannels(cameraFrame.channels());
		size_t sizeInBytes = cameraFrame.total() * cameraFrame.elemSize();
		kj::ArrayPtr<kj::byte> ff = kj::arrayPtr(cameraFrame.data, sizeInBytes);
		img.setData(ff);

		auto encoded_array = capnp::messageToFlatArray(message);
		auto encoded_array_ptr = encoded_array.asBytes();
		auto encoded_char_array = encoded_array_ptr.begin();
		auto size = encoded_array_ptr.size();
		zmq_msg_t msg;
		zmq_msg_init_data(&msg, encoded_char_array, size,NULL,NULL);
		zmq_msg_send(&msg,publisher,ZMQ_DONTWAIT);
		zmq_msg_close(&msg);
   }
	
	
	zmq_close(publisher);
	zmq_ctx_destroy(context);
	return 0;
}