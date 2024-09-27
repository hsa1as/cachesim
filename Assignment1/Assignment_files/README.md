# Introduction

This is the submission for course project - 1 : Cache simulator, CS6600 ( JUL-NOV 2024 ), IIT Madras.
Name: R Sai Ashwin
Roll: NS24Z344

# Building

The included Dockerfile was slightly modified to create a container with the code in `~/src/`. It copies the provided `ref_outputs` subdirectory from `Assignment1/Assignment_files/` into `~/src/ref_outputs` in the container.

The `src` subdirectory contains a Makefile

The makefile has 3 targets: all, run, and diff

To build the dockerfile and test the project:
1. Run `make clean` in `src/` to remove object files from the host filesystem
2. Run `docker build -t "cache_sim_ns24z344" .` in the root of the repository
3. Run `docker run -it cache_sim_ns24z344` to start the container and drop into a shell.
4. Run `make diff` to build the project, and view the difference between the provided files `ref_outputs/` and generated output files in `outputs/`

To build the project, `make all` can be used. This outputs a binary, `cache_sim`.

`make run` can be used to generate the outputs from the binary. Outputs are stored in `/src/outputs/`.

`make diff` can be run to compare the outputs in `/src/ref_outputs` with `/src/outputs`.

# Troubleshooting
## Linking error while building inside container
The most common cause for this is having compiled object files `main.o` and `cache.o` in `src/` on the host. Removing these object files and rebuilding the image should fix the issue

Run `make clean` in `src/` before building the docker image
