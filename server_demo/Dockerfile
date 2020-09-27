FROM ruby:2.6.0-alpine

RUN apk update && apk upgrade
WORKDIR /app
COPY Gemfile .
RUN bundle install && bundle clean

# application selection; main or setup
ARG APP_DIR='main'
COPY $APP_DIR .

CMD ["ruby", "app.rb", "-p", "80", "-o", "0.0.0.0"]

# command:
# docker build . -t ir-station-demo --build-arg APP_DIR=main
# docker run -itd -p 80:80 ir-station-demo
