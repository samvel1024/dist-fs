#include "commons.h"

template<typename T>
fd_guard<T>::fd_guard(int fd, T closer): fd(fd), closer(closer) {}

template<typename T>
fd_guard<T>::~fd_guard() {
	this->closer(this->fd);
}
