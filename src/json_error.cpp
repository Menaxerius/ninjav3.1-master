#include "json_error.hpp"

std::string json_error( const int err_num, const std::string &err_msg ) {
        return "{\"errorCode\":" + std::to_string(err_num) + ",\"errorDescription\":\"" + err_msg + "\"}";
}
