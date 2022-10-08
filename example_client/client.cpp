#include <iostream>
#include<cassert>
#include<zmq.h>
#include<opencv2/opencv.hpp>
#include<capnp/serialize.h>
#include"test.capnp.h"
using namespace std;


int main()
{
   
	void* context = zmq_ctx_new();
	void* subscriber = zmq_socket(context, ZMQ_SUB);
    int rc = zmq_connect(subscriber, "tcp://localhost:5557");
    assert(rc == 0);

	char* filter = "";

     
    while(true) {
        zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, filter, strlen(filter));
        
        string s;
        zmq_msg_t msg;
        cv::Mat image;
        assert(zmq_msg_init(&msg) == 0);
        zmq_msg_recv(&msg,subscriber,0);


        assert(zmq_msg_size(&msg) % sizeof(capnp::word) == 0);
        auto num_words = zmq_msg_size(&msg) / sizeof(capnp::word);

        std::cout << "The zmq_msg_size(&msg) is " << zmq_msg_size(&msg) << ", which are "  << num_words << " words."<< std::endl;

        if (reinterpret_cast<uintptr_t>(zmq_msg_data(&msg)) % sizeof(capnp::word) == 0)
        {
            std::cout << "The message is memory aligned." << std::endl;
        }
        else
        {
            std::cout << "The message is not memory aligned." << std::endl;
        }

        
        auto words = kj::heapArray<capnp::word>(zmq_msg_size(&msg)/ sizeof(capnp::word));
        memcpy(words.begin(), zmq_msg_data(&msg), words.asBytes().size());
        capnp::FlatArrayMessageReader f(words);
        Image::Reader fr = f.getRoot<Image>();
        int s_r = fr.getChannels();
        int f_r = fr.getHeight();
        int i_r = fr.getWidth();
        std::cout << "received: " << s_r << ", " << f_r << ", " << i_r << std::endl;

        std::cout << fr.getData().size() << std::endl;
        auto data = fr.getData();

        kj::byte buf[data.size()];
        memcpy(buf, data.begin(), sizeof(buf));
        cv::Mat gay2(i_r,f_r, CV_8UC3, buf);
        cv::imshow("Display Image gay2", gay2);
        if (cv::waitKey(1) == 27) {
            break;
        }
         zmq_msg_close(&msg);
        
        
    }
    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}