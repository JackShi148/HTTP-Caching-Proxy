This project implements a HTTP caching proxy server which can handle and process GET, POST and CONNECT requests.

This project is deployed with docker, running this app may need to accomplish the following steps: 

Before everything starts:
```
sudo apt-get install docker-compose
```

Firstly, go: 
```
cd docker-deploy/src
```
Then, go:
```
chmod o+x run.sh
```
Finally, go:
```
sudo docker-compose up
```
Now the HTTP caching proxy is running properly, you can change the proxy of your web brower to test its functionalities. 

Remember to check docker-deploy/src/proxy.log to see all the details behind the screen. 
