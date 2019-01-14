/*
 * ZeroTier SDK - Network Virtualization Everywhere
 * Copyright (C) 2011-2019  ZeroTier, Inc.  https://www.zerotier.com/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * --
 *
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial closed-source software that incorporates or links
 * directly against ZeroTier software without disclosing the source code
 * of your own application.
 */

/**
 * @file
 *
 * ZeroTier Socket API
 */

#include <string.h>

#include "lwip/sockets.h"
#include "Defs.hpp"
#include "libzt.h"
#include "Debug.hpp"

#ifdef SDK_JNI
	#include <jni.h>
#ifndef _MSC_VER
	//#include <sys/socket.h>
	//#include <sys/types.h>
	//#include <sys/select.h>
	//#include <sys/ioctl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
	void ss2zta(JNIEnv *env, struct sockaddr_storage *ss, jobject addr);
	void zta2ss(JNIEnv *env, struct sockaddr_storage *ss, jobject addr);
	void ztfdset2fdset(JNIEnv *env, int nfds, jobject src_ztfd_set, fd_set *dest_fd_set);
	void fdset2ztfdset(JNIEnv *env, int nfds, fd_set *src_fd_set, jobject dest_ztfd_set);
#ifdef __cplusplus
}
#endif
#endif

//////////////////////////////////////////////////////////////////////////////
// Socket API                                                               //
//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// lwIP prototypes copied from lwip/src/include/sockets.h
// Don't call these directly, call zts_* functions instead
int lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int lwip_bind(int s, const struct sockaddr *name, socklen_t namelen);
int lwip_shutdown(int s, int how);
int lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);
int lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);
int lwip_getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);
int lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);
int lwip_close(int s);
int lwip_connect(int s, const struct sockaddr *name, socklen_t namelen);
int lwip_listen(int s, int backlog);
int lwip_recv(int s, void *mem, size_t len, int flags);
int lwip_read(int s, void *mem, size_t len);
int lwip_recvfrom(int s, void *mem, size_t len, int flags,
      struct sockaddr *from, socklen_t *fromlen);
int lwip_send(int s, const void *dataptr, size_t size, int flags);
int lwip_sendmsg(int s, const struct msghdr *message, int flags);
int lwip_sendto(int s, const void *dataptr, size_t size, int flags,
    const struct sockaddr *to, socklen_t tolen);
int lwip_socket(int domain, int type, int protocol);
int lwip_write(int s, const void *dataptr, size_t size);
int lwip_writev(int s, const struct iovec *iov, int iovcnt);
int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
                struct timeval *timeout);
int lwip_ioctl(int s, long cmd, void *argp);
int lwip_fcntl(int s, int cmd, int val);

// Copied from lwip/src/include/sockets.h and renamed to prevent a name collision
// with system definitions
struct lwip_sockaddr {
  u8_t        sa_len;
  sa_family_t sa_family;
  char        sa_data[14];
};

//////////////////////////////////////////////////////////////////////////////
// ZeroTier Socket API                                                      //
//////////////////////////////////////////////////////////////////////////////

int zts_ready();

int zts_socket(int socket_family, int socket_type, int protocol)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_socket(socket_family, socket_type, protocol);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_socket(
	JNIEnv *env, jobject thisObj, jint family, jint type, jint protocol)
{
	return zts_socket(family, type, protocol);
}
#endif

int zts_connect(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
	if (!addr) {
		return ZTS_ERR_INVALID_ARG;
	}
	if (addrlen > (int)sizeof(struct sockaddr_storage) || addrlen < (int)sizeof(struct sockaddr_in)) {
		return ZTS_ERR_INVALID_ARG;
	}
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_connect(fd, addr, addrlen);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_connect(
	JNIEnv *env, jobject thisObj, jint fd, jobject addr)
{
	struct sockaddr_storage ss;
	zta2ss(env, &ss, addr);
	socklen_t addrlen = ss.ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
	return zts_connect(fd, (struct sockaddr *)&ss, addrlen);
}
#endif

int zts_bind(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
	if (!addr) {
		return ZTS_ERR_INVALID_ARG;
	}
	if (addrlen > (int)sizeof(struct sockaddr_storage) || addrlen < (int)sizeof(struct sockaddr_in)) {
		return ZTS_ERR_INVALID_ARG;
	}
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_bind(fd, addr, addrlen);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_bind(
	JNIEnv *env, jobject thisObj, jint fd, jobject addr)
{
	struct sockaddr_storage ss;
	zta2ss(env, &ss, addr);
	socklen_t addrlen = ss.ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
	return zts_bind(fd, (struct sockaddr*)&ss, addrlen);
}
#endif

int zts_listen(int fd, int backlog)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_listen(fd, backlog);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_listen(
	JNIEnv *env, jobject thisObj, jint fd, int backlog)
{
	return zts_listen(fd, backlog);
}
#endif

int zts_accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_accept(fd, addr, addrlen);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_accept(
	JNIEnv *env, jobject thisObj, jint fd, jobject addr, jint port)
{
	struct sockaddr_storage ss;
	socklen_t addrlen = sizeof(struct sockaddr_storage);
	int err = zts_accept(fd, (struct sockaddr *)&ss, &addrlen);
	ss2zta(env, &ss, addr);
	return err;
}
#endif

#if defined(__linux__)
int zts_accept4(int fd, struct sockaddr *addr, socklen_t *addrlen, int flags)
{
	return !zts_ready() ? ZTS_ERR_INVALID_OP : ZTS_ERR_INVALID_OP;
}
#endif
#ifdef SDK_JNI
#if defined(__linux__)
 JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_accept4(
	JNIEnv *env, jobject thisObj, jint fd, jobject addr, jint port, jint flags)
 {
	struct sockaddr_storage ss;
	socklen_t addrlen = sizeof(struct sockaddr_storage);
	int err = zts_accept4(fd, (struct sockaddr *)&ss, &addrlen, flags);
	ss2zta(env, &ss, addr);
	return err;
}
#endif
#endif

int zts_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_setsockopt(fd, level, optname, optval, optlen);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_setsockopt(
	JNIEnv *env, jobject thisObj, jint fd, jint level, jint optname, jobject optval)
{
	jclass c = (*env).GetObjectClass(optval);
	if (!c) {
		return ZTS_ERR_INVALID_OP;
	}
	int optval_int = -1;

	if (optname == ZTS_SO_BROADCAST
		|| optname == ZTS_SO_KEEPALIVE
		|| optname == ZTS_SO_REUSEADDR	
		|| optname == ZTS_SO_REUSEPORT
		|| optname == ZTS_TCP_NODELAY)
	{
		jfieldID fid = (*env).GetFieldID(c, "booleanValue", "B");
		optval_int = (int)(*env).GetBooleanField(optval, fid);
	}
	if (optname == ZTS_IP_TTL	
		|| optname == ZTS_IP_TOS
		|| optname == ZTS_SO_LINGER
		|| optname == ZTS_SO_RCVBUF
		|| optname == ZTS_SO_SNDBUF)
	{
		jfieldID fid = (*env).GetFieldID(c, "integerValue", "I");
		optval_int = (*env).GetIntField(optval, fid);
	}
	int optlen = sizeof(optval_int);
	return zts_setsockopt(fd, level, optname, &optval_int, optlen);
}
#endif

int zts_getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_getsockopt(fd, level, optname, optval, optlen);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_getsockopt(
	JNIEnv *env, jobject thisObj, jint fd, jint level, jint optname, jobject optval)
{
	jclass c = (*env).GetObjectClass(optval);
	if (!c) {
		return ZTS_ERR_INVALID_OP;
	}
	int optval_int;
	int optlen; // Intentionally not used
	int err = ZTS_ERR_OK;
	err = zts_getsockopt(fd, level, optname, &optval_int, &optlen);
	if (optname == ZTS_SO_BROADCAST
		|| optname == ZTS_SO_KEEPALIVE
		|| optname == ZTS_SO_REUSEADDR	
		|| optname == ZTS_SO_REUSEPORT
		|| optname == ZTS_TCP_NODELAY)
	{
		jfieldID fid = (*env).GetFieldID(c, "isBoolean", "B");
		(*env).SetBooleanField(optval, fid, true);
		fid = (*env).GetFieldID(c, "booleanValue", "B");
		(*env).SetBooleanField(optval, fid, (bool)optval_int);
	}
	if (optname == ZTS_IP_TTL	
		|| optname == ZTS_IP_TOS
		|| optname == ZTS_SO_LINGER
		|| optname == ZTS_SO_RCVBUF
		|| optname == ZTS_SO_SNDBUF)
	{
		jfieldID fid = (*env).GetFieldID(c, "isInteger", "B");
		(*env).SetBooleanField(optval, fid, true);
		fid = (*env).GetFieldID(c, "integerValue", "I");
		(*env).SetIntField(optval, fid, optval_int);
	}
	return err;
}
#endif

int zts_getsockname(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
	if (!addr) {
		return ZTS_ERR_INVALID_ARG;
	}
	if (*addrlen > (int)sizeof(struct sockaddr_storage) || *addrlen < (int)sizeof(struct sockaddr_in)) {
		return ZTS_ERR_INVALID_ARG;
	}
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_getsockname(fd, addr, addrlen);
}
#ifdef SDK_JNI
JNIEXPORT jboolean JNICALL Java_com_zerotier_libzt_ZeroTier_getsockname(JNIEnv *env, jobject thisObj,
	jint fd, jobject addr)
{
	struct sockaddr_storage ss;
	socklen_t addrlen = sizeof(struct sockaddr_storage);
	int err = zts_getsockname(fd, (struct sockaddr *)&ss, &addrlen);
	ss2zta(env, &ss, addr);
	return err;
}
#endif

int zts_getpeername(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
	if (!addr) {
		return ZTS_ERR_INVALID_ARG;
	}
	if (*addrlen > (int)sizeof(struct sockaddr_storage) || *addrlen < (int)sizeof(struct sockaddr_in)) {
		return ZTS_ERR_INVALID_ARG;
	}
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_getpeername(fd, addr, addrlen);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_getpeername(JNIEnv *env, jobject thisObj,
	jint fd, jobject addr)
{
	struct sockaddr_storage ss;
	int err = zts_getpeername(fd, (struct sockaddr *)&ss, (socklen_t *)sizeof(struct sockaddr_storage));
	ss2zta(env, &ss, addr);
	return err;
}
#endif

int zts_gethostname(char *name, size_t len)
{
	return !zts_ready() ? ZTS_ERR_INVALID_OP : ZTS_ERR_INVALID_OP; // TODO
}
#ifdef SDK_JNI
#endif

int zts_sethostname(const char *name, size_t len)
{
	return !zts_ready() ? ZTS_ERR_INVALID_OP : ZTS_ERR_INVALID_OP; // TODO
}
#ifdef SDK_JNI
#endif

struct hostent *zts_gethostbyname(const char *name)
{
	return (struct hostent *)(!zts_ready() ? NULL : NULL);
	// TODO: Test thread safety
	/*
	char buf[256];
	int buflen = 256;
	int h_err = 0;
	struct hostent hret;
	struct hostent **result = NULL;
	int err = 0;
	if ((err = lwip_gethostbyname_r(name, &hret, buf, buflen, result, &h_err)) != 0) {
		DEBUG_ERROR("err = %d", err);
		DEBUG_ERROR("h_err = %d", h_err);
		errno = h_err;
		return NULL; // failure
	}
	return *result;
	
	return lwip_gethostbyname(name);
	*/
}
#ifdef SDK_JNI
#endif

int zts_close(int fd)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_close(fd);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_close(
	JNIEnv *env, jobject thisObj, jint fd)
{
	return zts_close(fd);
}
#endif

int zts_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	struct timeval *timeout)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_select(nfds, readfds, writefds, exceptfds, timeout);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_select(JNIEnv *env, jobject thisObj,
	jint nfds, jobject readfds, jobject writefds, jobject exceptfds, jint timeout_sec, jint timeout_usec)
{
	struct timeval _timeout;
	_timeout.tv_sec  = timeout_sec;
		_timeout.tv_usec = timeout_usec;
		fd_set _readfds, _writefds, _exceptfds;   		
		fd_set *r = NULL;
		fd_set *w = NULL;
		fd_set *e = NULL;
		if (readfds) {
			r = &_readfds;
			ztfdset2fdset(env, nfds, readfds, &_readfds);
		}
		if (writefds) {
			w = &_writefds;
			ztfdset2fdset(env, nfds, writefds, &_writefds);
		}
		if (exceptfds) {
			e = &_exceptfds;
			ztfdset2fdset(env, nfds, exceptfds, &_exceptfds);
		}
	int err = zts_select(nfds, r, w, e, &_timeout);
	if (readfds) {
		fdset2ztfdset(env, nfds, &_readfds, readfds);
	}
	if (writefds) {
			fdset2ztfdset(env, nfds, &_writefds, writefds);
		}
		if (exceptfds) {
			fdset2ztfdset(env, nfds, &_exceptfds, exceptfds);
		}
	return err;
}
#endif

int zts_fcntl(int fd, int cmd, int flags)
{
	// translation from platform flag values to stack flag values
	int translated_flags = 0;
#if defined(__linux__)
	if (flags == 2048) {
		translated_flags = 1;
	}
#endif
#if defined(__APPLE__)
	if (flags == 4) {
		translated_flags = 1;
	}
#endif
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_fcntl(fd, cmd, translated_flags);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_fcntl(
	JNIEnv *env, jobject thisObj, jint fd, jint cmd, jint flags)
{
	return zts_fcntl(fd, cmd, flags);
}
#endif

int zts_ioctl(int fd, unsigned long request, void *argp)
{
	if (!argp) {
		return ZTS_ERR_INVALID_ARG;
	}
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_ioctl(fd, request, argp);
}
#ifdef SDK_JNI
JNIEXPORT int JNICALL Java_com_zerotier_libzt_ZeroTier_ioctl(
	JNIEnv *env, jobject thisObj, jint fd, jlong request, jobject argp)
{
	zts_err_t retval = ZTS_ERR_OK;
	if (request == FIONREAD) {
		DEBUG_ERROR("FIONREAD");
		int bytesRemaining = 0;
		retval = zts_ioctl(fd, request, &bytesRemaining);	
		// set value in general object
		jclass c = (*env).GetObjectClass(argp);
		if (!c) {
			return ZTS_ERR_INVALID_ARG;
		}
		jfieldID fid = (*env).GetFieldID(c, "integer", "I");
		(*env).SetIntField(argp, fid, bytesRemaining);
	}
	if (request == FIONBIO) {
		// TODO: double check
		int meaninglessVariable = 0;
		DEBUG_ERROR("FIONBIO");
		retval = zts_ioctl(fd, request, &meaninglessVariable);		
	}
	return retval;
}
#endif

ssize_t zts_send(int fd, const void *buf, size_t len, int flags)
{
	if (!buf || len <= 0) {
		return ZTS_ERR_INVALID_ARG;
	}
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_send(fd, buf, len, flags);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_send(
	JNIEnv *env, jobject thisObj, jint fd, jbyteArray buf, int flags)
{
	void *data = env->GetPrimitiveArrayCritical(buf, NULL);
	int w = zts_send(fd, data, env->GetArrayLength(buf), flags);
	env->ReleasePrimitiveArrayCritical(buf, data, 0);
	return w;
}
#endif

ssize_t zts_sendto(int fd, const void *buf, size_t len, int flags, 
	const struct sockaddr *addr, socklen_t addrlen)
{
	if (!addr || !buf || len <= 0) {
		return ZTS_ERR_INVALID_ARG;
	}
	if (addrlen > (int)sizeof(struct sockaddr_storage) || addrlen < (int)sizeof(struct sockaddr_in)) {
		return ZTS_ERR_INVALID_ARG;
	}
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_sendto(fd, buf, len, flags, addr, addrlen);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_sendto(
	JNIEnv *env, jobject thisObj, jint fd, jbyteArray buf, jint flags, jobject addr)
{
	void *data = env->GetPrimitiveArrayCritical(buf, NULL);
	struct sockaddr_storage ss;
	zta2ss(env, &ss, addr);
	socklen_t addrlen = ss.ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
	int w = zts_sendto(fd, data, env->GetArrayLength(buf), flags, (struct sockaddr *)&ss, addrlen);
	env->ReleasePrimitiveArrayCritical(buf, data, 0);
	return w;
}
#endif

ssize_t zts_sendmsg(int fd, const struct msghdr *msg, int flags)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_sendmsg(fd, msg, flags);
}
#ifdef SDK_JNI
#endif

ssize_t zts_recv(int fd, void *buf, size_t len, int flags)
{
	if (!buf) {
		return ZTS_ERR_INVALID_ARG;
	}
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_recv(fd, buf, len, flags);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_recv(JNIEnv *env, jobject thisObj,
	jint fd, jbyteArray buf, jint flags)
{
	void *data = env->GetPrimitiveArrayCritical(buf, NULL);
	int r = zts_recv(fd, data, env->GetArrayLength(buf), flags);
	env->ReleasePrimitiveArrayCritical(buf, data, 0);
	return r;
}
#endif

ssize_t zts_recvfrom(int fd, void *buf, size_t len, int flags, 
	struct sockaddr *addr, socklen_t *addrlen)
{
	if (!buf) {
		return ZTS_ERR_INVALID_ARG;
	}
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_recvfrom(fd, buf, len, flags, addr, addrlen);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_recvfrom(
	JNIEnv *env, jobject thisObj, jint fd, jbyteArray buf, jint flags, jobject addr)
{
	socklen_t addrlen = sizeof(struct sockaddr_storage);
	struct sockaddr_storage ss;
	void *data = env->GetPrimitiveArrayCritical(buf, NULL);
	int r = zts_recvfrom(fd, data, env->GetArrayLength(buf), flags, (struct sockaddr *)&ss, &addrlen);
	env->ReleasePrimitiveArrayCritical(buf, data, 0);
	ss2zta(env, &ss, addr);
	return r;
}
#endif

ssize_t zts_recvmsg(int fd, struct msghdr *msg, int flags)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : -1; // Not currently implemented by stack
}
#ifdef SDK_JNI
#endif

int zts_read(int fd, void *buf, size_t len)
{
	if (!buf) {
		return ZTS_ERR_INVALID_ARG;
	}
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_read(fd, buf, len);
}
int zts_read_offset(int fd, void *buf, size_t offset, size_t len)
{
	if (!buf) {
		return ZTS_ERR_INVALID_ARG;
	}
	char *cbuf = (char*)buf;
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_read(fd, &(cbuf[offset]), len);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_read(JNIEnv *env, jobject thisObj,
	jint fd, jbyteArray buf)
{
	void *data = env->GetPrimitiveArrayCritical(buf, NULL);
	int r = zts_read(fd, data, env->GetArrayLength(buf));
	env->ReleasePrimitiveArrayCritical(buf, data, 0);
	return r;
}
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_read_1offset(JNIEnv *env, jobject thisObj,
	jint fd, jbyteArray buf, jint offset, jint len)
{
	void *data = env->GetPrimitiveArrayCritical(buf, NULL);
	int r = zts_read_offset(fd, data, offset, len);
	env->ReleasePrimitiveArrayCritical(buf, data, 0);
	return r;
}
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_read_1length(JNIEnv *env, jobject thisObj,
	jint fd, jbyteArray buf, jint len)
{
	void *data = env->GetPrimitiveArrayCritical(buf, NULL);
	int r = zts_read(fd, data, len);
	env->ReleasePrimitiveArrayCritical(buf, data, 0);
	return r;
}
#endif

int zts_write(int fd, const void *buf, size_t len)
{
	if (!buf || len <= 0) {
		return ZTS_ERR_INVALID_ARG;
	}
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_write(fd, buf, len);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_write__IB(JNIEnv *env, jobject thisObj,
	jint fd, jbyteArray buf)
{
	void *data = env->GetPrimitiveArrayCritical(buf, NULL);
	int w = zts_write(fd, data, env->GetArrayLength(buf));
	env->ReleasePrimitiveArrayCritical(buf, data, 0);
	return w;
}
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_write_1offset(JNIEnv *env, jobject thisObj,
	jint fd, jbyteArray buf, jint offset, jint len)
{
	void *data = env->GetPrimitiveArrayCritical(&(buf[offset]), NULL); // PENDING: check?
	int w = zts_write(fd, data, len);
	env->ReleasePrimitiveArrayCritical(buf, data, 0);
	return w;
}
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_write_1byte(JNIEnv *env, jobject thisObj,
	jint fd, jbyte buf)
{
	return zts_write(fd, &buf, 1);
}
#endif

int zts_shutdown(int fd, int how)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : lwip_shutdown(fd, how);
}
#ifdef SDK_JNI
JNIEXPORT jint JNICALL Java_com_zerotier_libzt_ZeroTier_shutdown(
	JNIEnv *env, jobject thisObj, int fd, int how)
{
	return zts_shutdown(fd, how);
}
#endif

int zts_add_dns_nameserver(struct sockaddr *addr)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : -1; // TODO
}
#ifdef SDK_JNI
#endif

int zts_del_dns_nameserver(struct sockaddr *addr)
{
	return !zts_ready() ? ZTS_ERR_SERVICE : -1; // TODO
}
#ifdef SDK_JNI
#endif

#ifdef SDK_JNI
void ztfdset2fdset(JNIEnv *env, int nfds, jobject src_ztfd_set, fd_set *dest_fd_set)
{
	jclass c = (*env).GetObjectClass(src_ztfd_set);
	if (!c) {
		return;
	}
	FD_ZERO(dest_fd_set);
	jfieldID fid = env->GetFieldID(c, "fds_bits", "[B");
	jobject fdData = (*env).GetObjectField (src_ztfd_set, fid);
	jbyteArray * arr = reinterpret_cast<jbyteArray*>(&fdData);
	char *data = (char*)(*env).GetByteArrayElements(*arr, NULL);	
	for (int i=0; i<nfds; i++) {
		if (data[i] == 0x01)  {
			FD_SET(i, dest_fd_set);
		}
	}
	(*env).ReleaseByteArrayElements(*arr, (jbyte*)data, 0);
	return;
}

void fdset2ztfdset(JNIEnv *env, int nfds, fd_set *src_fd_set, jobject dest_ztfd_set)
{
	jclass c = (*env).GetObjectClass(dest_ztfd_set);
	if (!c) {
		return;
	}
	jfieldID fid = env->GetFieldID(c, "fds_bits", "[B");
	jobject fdData = (*env).GetObjectField (dest_ztfd_set, fid);
	jbyteArray * arr = reinterpret_cast<jbyteArray*>(&fdData);
	char *data = (char*)(*env).GetByteArrayElements(*arr, NULL);
	for (int i=0; i<nfds; i++) {
		if (FD_ISSET(i, src_fd_set)) {
			data[i] = 0x01;
		}
	}
	(*env).ReleaseByteArrayElements(*arr, (jbyte*)data, 0);
	return;
}

//////////////////////////////////////////////////////////////////////////////
// Helpers (for moving data across the JNI barrier)                         //
//////////////////////////////////////////////////////////////////////////////

void ss2zta(JNIEnv *env, struct sockaddr_storage *ss, jobject addr)
{
	jclass c = (*env).GetObjectClass(addr);
	if (!c) {
		return;
	}
	if(ss->ss_family == AF_INET)
	{
		struct sockaddr_in *in4 = (struct sockaddr_in*)ss;
		jfieldID fid = (*env).GetFieldID(c, "_port", "I");
		(*env).SetIntField(addr, fid, ntohs(in4->sin_port));
		fid = (*env).GetFieldID(c,"_family", "I");
		(*env).SetIntField(addr, fid, (in4->sin_family));
		fid = env->GetFieldID(c, "_ip4", "[B");
		jobject ipData = (*env).GetObjectField (addr, fid);
		jbyteArray * arr = reinterpret_cast<jbyteArray*>(&ipData);
		char *data = (char*)(*env).GetByteArrayElements(*arr, NULL);
		memcpy(data, &(in4->sin_addr.s_addr), 4);
		(*env).ReleaseByteArrayElements(*arr, (jbyte*)data, 0);

		return;
	}
	if(ss->ss_family == AF_INET6)
	{
		struct sockaddr_in6 *in6 = (struct sockaddr_in6*)ss;
		jfieldID fid = (*env).GetFieldID(c, "_port", "I");
		(*env).SetIntField(addr, fid, ntohs(in6->sin6_port));
		fid = (*env).GetFieldID(c,"_family", "I");
		(*env).SetIntField(addr, fid, (in6->sin6_family));
		fid = env->GetFieldID(c, "_ip6", "[B");
		jobject ipData = (*env).GetObjectField (addr, fid);
		jbyteArray * arr = reinterpret_cast<jbyteArray*>(&ipData);
		char *data = (char*)(*env).GetByteArrayElements(*arr, NULL);
		memcpy(data, &(in6->sin6_addr.s6_addr), 16);
		(*env).ReleaseByteArrayElements(*arr, (jbyte*)data, 0);
		return;
	}
}

void zta2ss(JNIEnv *env, struct sockaddr_storage *ss, jobject addr)
{
	jclass c = (*env).GetObjectClass(addr);
	if (!c) {
		return;
	}
	jfieldID fid = (*env).GetFieldID(c, "_family", "I");
	int family = (*env).GetIntField(addr, fid);
	if (family == AF_INET)
	{
		struct sockaddr_in *in4 = (struct sockaddr_in*)ss;
		fid = (*env).GetFieldID(c, "_port", "I");
		in4->sin_port = htons((*env).GetIntField(addr, fid));
		in4->sin_family = AF_INET;
		fid = env->GetFieldID(c, "_ip4", "[B");
		jobject ipData = (*env).GetObjectField (addr, fid);
		jbyteArray * arr = reinterpret_cast<jbyteArray*>(&ipData);
		char *data = (char*)(*env).GetByteArrayElements(*arr, NULL);
		memcpy(&(in4->sin_addr.s_addr), data, 4);
		(*env).ReleaseByteArrayElements(*arr, (jbyte*)data, 0);
		return;
	}
	if (family == AF_INET6)
	{
		struct sockaddr_in6 *in6 = (struct sockaddr_in6*)ss;
		jfieldID fid = (*env).GetFieldID(c, "_port", "I");
		in6->sin6_port = htons((*env).GetIntField(addr, fid));
		fid = (*env).GetFieldID(c,"_family", "I");
		in6->sin6_family = AF_INET6;
		fid = env->GetFieldID(c, "_ip6", "[B");
		jobject ipData = (*env).GetObjectField (addr, fid);
		jbyteArray * arr = reinterpret_cast<jbyteArray*>(&ipData);
		char *data = (char*)(*env).GetByteArrayElements(*arr, NULL);
		memcpy(&(in6->sin6_addr.s6_addr), data, 16);
		(*env).ReleaseByteArrayElements(*arr, (jbyte*)data, 0);
		return;
	}
}
#endif // JNI

#ifdef __cplusplus
}
#endif
