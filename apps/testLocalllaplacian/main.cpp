#include <iostream>
#include "static_image.h"
#include "image_io.h"
#include "local_laplacian_cpu.h"
#include "local_laplacian_gpu.h"
#include "StaticDispatch.h"
#include "clock.h"
#include <limits>
#include <iomanip>

#ifndef DBL_MAX
#define DBL_MAX 1.79769e+308
#endif
#define CPU 1
#define GPU 2
#define Fus 3

using namespace std;
using namespace Fusion::Static;
inline void testPerformance(int type,int levels,float alpha,float beta,buffer_t* input,buffer_t* output) {
    StaticDispatch<int32_t,float,float> fusion(local_laplacian_cpu,local_laplacian_gpu,input,output);

    double bestT =DBL_MAX;
    double worstT=0;
    switch (type) {
    case CPU:
        fusion.realizeCPU(levels,alpha/(levels-1),beta);
        for (int i = 0; i < 5; i++) {
            double t1=current_time();
            fusion.realizeCPU(levels,alpha/(levels-1),beta);
            double t2=current_time();
            double t=t2-t1;
            if (t < bestT) bestT = t;
            if (t > worstT) worstT = t;
        }
        cout<<setw(15)<<"Best CPU: "<<setw(10)<<bestT<<setw(15)<<" Worst CPU: "<<setw(10)<<worstT<<endl;
        break;
    case GPU:
        fusion.realizeGPU(levels,alpha/(levels-1),beta);
        for (int i = 0; i < 5; i++) {
            double t1=current_time();
            fusion.realizeGPU(levels,alpha/(levels-1),beta);
            double t2=current_time();
            double t=t2-t1;
            if (t < bestT) bestT = t;
            if (t > worstT) worstT = t;
        }
        cout<<setw(15)<<"Best GPU: "<<setw(10)<<bestT<<setw(15)<<" Worst GPU: "<<setw(10)<<worstT<<endl;
        break;
    case Fus:
        fusion.realize(levels,alpha/(levels-1),beta,142);
        for (int i = 0; i < 5; i++) {
            double t1=current_time();
            fusion.realize(levels,alpha/(levels-1),beta,50);
            double t2=current_time();
            double t=t2-t1;
            if (t < bestT) bestT = t;
            if (t > worstT) worstT = t;
        }
        cout<<setw(15)<<"Best Fusion: "<<setw(10)<<bestT<<setw(15)<<" Worst Fusion: "<<setw(10)<<worstT<<endl;
        break;
    }
}


int main(int argc,char** argv) {
    if (argc < 6)
    {
        printf("Usage: ./process input.png levels alpha beta output.png\n"
               "e.g.: ./process input.png 8 1 1 output.png\n");
        return 0;
    }
    Image<uint16_t> input = load<uint16_t>(argv[1]);
    Image<uint16_t> output(input.width(),input.height(),input.channels());
    cout<<"Image Size : "<<input.width()<<" X "<<input.height()<<" X "<<input.channels()<<endl;
    int levels = atoi(argv[2]);
    float alpha = atof(argv[3]), beta = atof(argv[4]);
    StaticDispatch<int32_t,float,float> fusion(local_laplacian_cpu,local_laplacian_gpu,input,output);



    testPerformance(CPU,levels,alpha/(levels-1),beta,(buffer_t*)(input),output);
    testPerformance(GPU,levels,alpha/(levels-1),beta,(buffer_t*)(input),output);
    testPerformance(Fus,levels,alpha/(levels-1),beta,input,output);

    fusion.realize(levels,alpha/(levels-1),beta,360);
    save(output, argv[5]);

    return 0;

}
