FROM gcc:9

RUN mkdir /var/log/erss
RUN mkdir /app
WORKDIR /app
add . /app/

RUN chmod o+x run.sh
ENTRYPOINT ["./run.sh"]
