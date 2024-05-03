FROM public.ecr.aws/lambda/provided:al2-arm64
RUN yum install -y procps
RUN yum install -y iproute net-tools
COPY ./build /root/
COPY ./events /root/
