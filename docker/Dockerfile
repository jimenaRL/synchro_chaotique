FROM python:2.7-jessie

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt
RUN rm requirements.txt

RUN git clone https://github.com/jimenaRL/synchro_chaotique.git /usr/src/synchro_chaotique

WORKDIR /usr/src/synchro_chaotique/src

