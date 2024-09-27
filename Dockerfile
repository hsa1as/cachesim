FROM ubuntu:22.04
RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install -y curl
RUN apt-get install -y g++
RUN apt-get install -y build-essential
WORKDIR /app
COPY ./src/ ./
COPY ./Assignment1/Assignment_files/ref_outputs ./ref_outputs


