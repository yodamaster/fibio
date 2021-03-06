//
//  test_http_client.cpp
//  fibio
//
//  Created by Chen Xu on 14-3-12.
//  Copyright (c) 2014 0d0a.com. All rights reserved.
//

#include <iostream>
#include <vector>
#include <sstream>
#include <fibio/fiber.hpp>
#include <fibio/http/client/client.hpp>
#include <fibio/fiberize.hpp>

using namespace fibio;
using namespace fibio::http;

void the_client() {
    client c;
    client::request req;
    req.url("/");
    req.headers.insert(std::make_pair("Host", "0d0a.com"));
    // Default method
    assert(req.method()==http_method::INVALID_METHOD);
    req.method(http_method::GET);
    // Default version
    assert(req.version()==http_version::INVALID_VERSION);
    req.version(http_version::HTTP_1_1);
    assert(req.content_length()==0);
    //c.set_auto_decompress(true);
    
    c.connect("0d0a.com");
    for(int i=0; i<10; i++) {
        client::response resp;
        if(c.send_request(req, resp)) {
            // This server returns a 200 response
            assert(resp.version==http_version::HTTP_1_1);
            assert(resp.status_code==http_status_code::OK);

            size_t cl=resp.content_length;
            std::string s;
            std::stringstream ss;
            ss << resp.body_stream().rdbuf();
            s=ss.str();
            assert(s.size()==cl);
            // Make sure we triggered EOF condition
            resp.body_stream().peek();
            assert(resp.body_stream().eof());
        }
    }
}

void the_url_client() {
    url_client uc;
    {
        client::response &resp=uc.request("http://0d0a.com/");
        assert(resp.status_code==http_status_code::OK);
    }
    {
        client::response &resp=uc.request("http://www.baidu.com/");
        assert(resp.status_code==http_status_code::OK);
    }
    {
        // http://github.com/ responses with 301, redirects to 'https://github.com/'
        client::response &resp=uc.request("http://github.com/");
        assert(resp.status_code==http_status_code::OK);
    }
}

int fibio::main(int argc, char *argv[]) {
    fiber_group fibers;
    for (int i=0; i<1; i++) {
        fibers.create_fiber(the_client);
        fibers.create_fiber(the_url_client);
    }
    fibers.join_all();
    std::cout << "main_fiber exiting" << std::endl;
    return 0;
}
