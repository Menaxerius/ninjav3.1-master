#ifndef INCLUDE__REQRESP_HPP
#define INCLUDE__REQRESP_HPP

#include "Request.hpp"
#include "Response.hpp"

class ReqResp {
	public:
		Request						*req;
		Response					*resp;
		
		ReqResp( Request *req_in, Response *resp_in) : req(req_in), resp(resp_in) { };
};

#endif
