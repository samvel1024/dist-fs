#ifndef DISTFS_COMMONS_H
#define DISTFS_COMMONS_H

typedef int(*fd_closer)(int);

template<typename T>
class fd_guard {
private:
	int fd;
	T closer;
public:
	fd_guard(int fd, T closer);

	virtual ~fd_guard();
};


#endif //DISTFS_COMMONS_H
