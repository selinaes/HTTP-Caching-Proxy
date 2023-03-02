FROM ubuntu:18.04
RUN apt-get update && apt-get install -y g++
RUN mkdir /code
RUN mkdir /var/log/erss
RUN chmod 777 /var/log/erss
ADD . /code
WORKDIR /code
RUN chmod +x run.sh

