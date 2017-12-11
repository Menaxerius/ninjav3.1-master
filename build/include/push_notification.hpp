#ifndef INCLUDE__PUSH_NOTIFICATION_HPP
#define INCLUDE__PUSH_NOTIFICATION_HPP

#include <string>

void push_notification( const std::string &url, const std::string &key, const std::string &user, const std::string &message, const std::string &sound = "" );

#endif
