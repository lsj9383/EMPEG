#include<string>
//class string;
#include<exception>
//class string;					//�����������
#ifndef _EMPEGEXCEPTION_H
#define _EMPEGEXCEPTION_H

//--------���װ���쳣---------------------------
class OpenException :public std::exception
{
public:
	OpenException(std::string w, std::string r = '\0') :_reson(r), _where(w){};
	OpenException(std::string w, AVFormatContext* avformatCtx) :_avformatCtx(avformatCtx), _where(w){};
	OpenException(std::string w, AVCodecContext*  avcodecCtc)  :_avcodecCtc(avcodecCtc), _where(w){};
	virtual const std::string& what() { return _reson; };		//�쳣ԭ��
	virtual const std::string& where(){ return _where; };		//�쳣λ��
private:
	std::string _reson;
	std::string _where;
	AVFormatContext* _avformatCtx = nullptr;
	AVCodecContext*  _avcodecCtc = nullptr;
};

//-------���������쳣--------------------------
class StreamExceptionPara :public std::exception
{
public:
	StreamExceptionPara(std::string w, std::string r = '\0') :_reson(r), _where(w){};
	StreamExceptionPara(std::string w, AVFormatContext* avformatCtx) :_avformatCtx(avformatCtx), _where(w){};
	StreamExceptionPara(std::string w, AVCodecContext*  avcodecCtc)  :_avcodecCtc(avcodecCtc),   _where(w){};
	virtual const std::string& what() { return _reson; };		//�쳣ԭ��
	virtual const std::string& where(){ return _where; };		//�쳣λ��

private:
	std::string _reson;
	std::string _where;
	AVFormatContext* _avformatCtx = nullptr;
	AVCodecContext*  _avcodecCtc = nullptr;
};

//-------д�����쳣--------------------------
class WriteExceptionPara :public std::exception
{
public:
	WriteExceptionPara(std::string w, std::string r = '\0') :_reson(r), _where(w){};
	WriteExceptionPara(std::string w, AVFormatContext* avformatCtx) :_avformatCtx(avformatCtx), _where(w){};
	WriteExceptionPara(std::string w, AVCodecContext*  avcodecCtc)  :_avcodecCtc(avcodecCtc), _where(w){};
	WriteExceptionPara(AVFormatContext* avformatCtx, AVPacket* packet) :_avformatCtx(avformatCtx), _packet(packet){};
	virtual const std::string& what() { return _reson; };		//�쳣ԭ��
	virtual const std::string& where(){ return _where; };		//�쳣λ��

private:
	std::string _reson;
	std::string _where;
	AVFormatContext* _avformatCtx;// = nullptr;
	AVCodecContext*  _avcodecCtc;// = nullptr;
	AVPacket*		 _packet;// = nullptr;
};

//-------������쳣--------------------------
class DecodeExceptionPara :public std::exception
{
public:
	DecodeExceptionPara(std::string w, std::string r = '\0') :_reson(r), _where(w){};
	//DecodeExceptionPara(std::string w, std::string r = '\0') :_reson(r), _where(w){};
	virtual const std::string& what() { return _reson; };		//�쳣ԭ��
	virtual const std::string& where(){ return _where; };		//�쳣λ��

private:
	std::string _reson;
	std::string _where;
};

//-------�����쳣--------------------------
class ParamExceptionPara :public std::exception
{
public:
	ParamExceptionPara(std::string w, std::string r = '\0') :_reson(r), _where(w){};
	virtual const std::string& what() { return _reson; };		//�쳣ԭ��
	virtual const std::string& where(){ return _where; };		//�쳣λ��

private:
	std::string _reson;
	std::string _where;
};
#endif