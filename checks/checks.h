
#ifndef CHECKS_H
#define CHECKS_H


int check_db();
int check_io_file();
int check_io_buffered();
int check_io_socket();
int check_io_tls();
int check_io_http();
int check_io_https();
int check_io_packets();

int check_syncio_tls();
int check_syncio_http();

#endif

