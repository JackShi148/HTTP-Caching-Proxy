This project implements a HTTP caching proxy server which can handle and process GET, POST and CONNECT requests.

This project is deployed with docker, running this app may need to accomplish the following steps: 

Firstly, go: 
```
cd docker-depoly/src
```
Then, go:
```
chmod o+x run.sh
```
Finally, go:
```
sudo docker-compose up
```
Now the HTTP caching proxy is running properly, you can change the proxy of your web brower to test its functionality. 
