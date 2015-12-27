bin_PROGRAMS=echo_server echo_client

#echo_client_SOURCES=echo_client.cc connection.cc buffer.cc poller.cc networking.cc tcp_client.cc 
echo_client_SOURCES=connection.cc buffer.cc poller.cc socket.cc echo_client.cc connector.cc tcp_connection.cc
echo_server_SOURCES=echo_server.cc connection.cc buffer.cc poller.cc socket.cc tcp_connection.cc

AM_CPPFLAGS=-I/home/zhangguanxing/staging/include -O0 -g -DLL_DEBUG
AM_LDFLAGS=-L/home/zhangguanxing/staging/lib -lglog
