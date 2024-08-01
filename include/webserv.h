#pragma once

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __APPLE__
#include <sys/epoll.h>
#endif
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#include "Logger.hpp"
#include "Config.hpp"

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100
#define PORT 8081
