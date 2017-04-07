#include <iostream>
#include <opencv2/opencv.hpp>

#include "vpg.h"

template <typename T>
std::string num2str(T value, unsigned char precision=1);

void drawDataWindow(const cv::String &_title, const cv::Size _windowsize, const double *_data, const int _datalength, double _ymax, double _ymin, cv::Scalar _color);

int main(int argc, char *argv[])
{      
    // Try to open video capture device by Opencv's API
    cv::VideoCapture capture;
    if(capture.open(0)) {

        // Create FaceProcessor instance
        cv::String facecascadefilename;
        #ifdef CASCADE_FILENAME
            facecascadefilename = cv::String(CASCADE_FILENAME) + "haarcascade_frontalface_alt2.xml";
        #else
            facecascadefilename = "haarcascade_frontalface_alt2.xml";
        #endif                      
        vpg::FaceProcessor faceproc(facecascadefilename);
        if(faceproc.empty()) {
            std::cout << "Could not load cascade classifier! Abort...\n";
            return -1;
        }

        double framePeriod = faceproc.measureFramePeriod(&capture); // ms
        vpg::PulseProcessor pulseproc(framePeriod); // note it is convinirnt to use default constructor only if frame period is near to 33 ms

        // Add peak detector for the cardio intervals evaluation and analysis
        int totalcardiointervals = 75;
        vpg::PeakDetector peakdetector(pulseproc.getLength(), totalcardiointervals, 11, framePeriod);
        pulseproc.setPeakDetector(&peakdetector);

        // Add HRVProcessor for HRV analysis
        vpg::HRVProcessor hrvproc;

        // Create local variables to store frame and processing values
        cv::Mat frame;
        unsigned int k = 0;
        double s = 0.0, t = 0.0, _snr = 0.0;
        int _hr = 80;

        const double *signal = pulseproc.getSignal();
        int length = pulseproc.getLength();

        const double *cardiointervals = peakdetector.getIntervalsVector();
        int cardiointervalslength = peakdetector.getIntervalsLength();
        const double *binarysignal = peakdetector.getBinarySignal();

        std::cout << "Press escape to exit..." << std::endl;

        // Start video processing cycle
        while(true) {
            if(capture.read(frame)) {

                // Essential part for the PPG signal extraction, only 2 strings should be called for the each new frame
                faceproc.enrollImage(frame, s, t);
                pulseproc.update(s,t);

                // Draw hr
                cv::String _hrstr = std::to_string(_hr) + " bpm (snr: " + num2str(_snr,2) + " dB)";
                cv::putText(frame, _hrstr, cv::Point(20,40), CV_FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0,0,0), 1, CV_AA);
                cv::putText(frame, _hrstr, cv::Point(19,39), CV_FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255,255,255), 1, CV_AA);

                // Draw current cardio interval
                cv::String _cardiointerval = num2str(peakdetector.getCurrentInterval(),0) + " ms";
                cv::putText(frame, _cardiointerval, cv::Point(20,70), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
                cv::putText(frame, _cardiointerval, cv::Point(19,69), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127,0,127), 1, CV_AA);

                // Draw cardio intervals history
                float xorigin = 10.0;
                float yorigin = frame.rows;
                float xstep = (float)(frame.cols - xorigin*2.0f) / (cardiointervalslength - 1);
                float ystep = (float)(frame.rows) / 4000.0f;
                for(int i = 0; i < cardiointervalslength-1; i++) {
                    cv::line(frame, cv::Point(xorigin + i*xstep, yorigin - cardiointervals[i]*ystep), cv::Point(xorigin + (i+1)*xstep, yorigin - cardiointervals[(i+1)]*ystep),cv::Scalar(127,0,127),1,CV_AA);
                    cv::line(frame, cv::Point(xorigin + i*xstep, yorigin), cv::Point(xorigin + i*xstep, yorigin - cardiointervals[i]*ystep),cv::Scalar(127,0,127),1,CV_AA);
                }
                cv::line(frame, cv::Point(xorigin + (cardiointervalslength-1)*xstep, yorigin), cv::Point(xorigin + (cardiointervalslength-1)*xstep, yorigin - cardiointervals[cardiointervalslength-1]*ystep),cv::Scalar(127,0,127),1,CV_AA);

                // Draw ppg-signal
                xstep = (float)(frame.cols - xorigin*2.0f) / (length - 1);
                ystep *= 100.0;
                yorigin = (frame.rows) * 0.9f;
                for(int i = 0; i < length-1; i++)
                    cv::line(frame, cv::Point(xorigin + i*xstep, yorigin - signal[i]*ystep), cv::Point(xorigin + (i+1)*xstep, yorigin - signal[(i+1)]*ystep),cv::Scalar(0,200,0),1,CV_AA);

                // Draw binary-signal from PeakDetector
                for(int i = 0; i < length-1; i++)
                    cv::line(frame, cv::Point(xorigin + i*xstep, yorigin - binarysignal[i]*ystep), cv::Point(xorigin + (i+1)*xstep, yorigin - binarysignal[(i+1)]*ystep),cv::Scalar(0,0,255),1,CV_AA);

                // Draw frame time
                cv::String _periodstr = num2str(t) + " ms";
                cv::putText(frame, _periodstr, cv::Point(20,frame.rows-10), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
                cv::putText(frame, _periodstr, cv::Point(19,frame.rows-11), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, CV_AA);

                cv::imshow("Video probe", frame);
            }
            // After frame processing We could want to evaluate heart rate, here at each 32-th frame
            if(k % 33 == 0) {
                _hr = (int)pulseproc.computeFrequency();
                _snr = pulseproc.getSNR();
            }

            // Evaluate and draw HRV signal
            if(k % 5 == 0) {
                hrvproc.enrollIntervals(peakdetector.getIntervalsVector(), peakdetector.getIntervalsLength());
                drawDataWindow("HRV Signal, [milliseconds]",cv::Size(640,480),hrvproc.getHRVSignal(),hrvproc.getHRVSignalLength(), 1500.0, 0.0, cv::Scalar(0,127,255));
                //drawDataWindow("Amplitude Fourier spectrum of HRV Signal",cv::Size(640,480),hrvproc.getHRVAmplitudeSpectrum(),hrvproc.getHRVAmplitudeSpectrumLength(), 2.0e3, 0.0, cv::Scalar(0,0,255));
            }

            k++;
            // Process keyboard events
            int c = cv::waitKey(1);
            if( (char)c == 27 ) // 27 is escape ASCII code
                break;
        }
        capture.release();
    } else {
        std::cerr << "Can not open video capture device. Abort...";
        return -1;
    }
    return 0;
}

template <typename T>
std::string num2str(T value, unsigned char precision)
{
    if(precision > 0)
        return std::to_string(static_cast<long>(value)) + "." + std::to_string( static_cast<long>(std::abs(value - static_cast<long>(value))*std::pow(10.0,precision)) );
    else
        return std::to_string(static_cast<long>(value));
}

void drawDataWindow(const cv::String &_title, const cv::Size _windowsize, const double *_data, const int _datalength, double _ymax, double _ymin, cv::Scalar _color)
{
    if(_datalength > 0 && _windowsize.area() > 0 && _data != NULL ) {

        cv::Mat _colorplot = cv::Mat::zeros(_windowsize, CV_8UC3);
        cv::rectangle(_colorplot,cv::Rect(0,0,_colorplot.cols,_colorplot.rows),cv::Scalar(15,15,15), -1);

        int _ticksX = 10;
        double _tickstepX = static_cast<double>(_windowsize.width)/ _ticksX ;
        for(int i = 1; i < _ticksX ; i++)
            cv::line(_colorplot, cv::Point2f(i*_tickstepX,0), cv::Point2f(i*_tickstepX,_colorplot.rows), cv::Scalar(100,100,100), 1);

        int _ticksY = 8;
        double _tickstepY = static_cast<double>(_windowsize.height)/ _ticksY ;
        for(int i = 1; i < _ticksY ; i++) {
            cv::line(_colorplot, cv::Point2f(0,i*_tickstepY), cv::Point2f(_colorplot.cols,i*_tickstepY), cv::Scalar(100,100,100), 1);
            cv::putText(_colorplot, num2str(_ymax - i * (_ymax-_ymin)/_ticksY), cv::Point(5, i*_tickstepY - 10), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(150,150,150), 1, CV_AA);
        }

        double invstepY = (_ymax - _ymin) / _windowsize.height;
        double stepX = static_cast<double>(_windowsize.width) / (_datalength - 1);

        for(int i = 0; i < _datalength - 1; i++) {
            cv::line(_colorplot, cv::Point2f(i*stepX, _windowsize.height - (_data[i] - _ymin)/invstepY),
                                 cv::Point2f((i+1)*stepX, _windowsize.height - (_data[i+1] - _ymin)/invstepY),
                                 _color, 1, CV_AA);
        }
        cv::imshow(_title, _colorplot);
    }
}