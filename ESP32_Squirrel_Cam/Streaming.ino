const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
                      "Access-Control-Allow-Origin: *\r\n" \
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
const int hdrLen = strlen(HEADER);
const int bdrLen = strlen(BOUNDARY);
const int cntLen = strlen(CTNTTYPE);
WiFiClient streamingClient;
void handle_jpg_stream(void)
{
  PrintMessageLn(F("Handle Jpg-Stream"));

  streamingClient = webServer.client();
  streamingClient.write(HEADER, hdrLen);
  streamingClient.write(BOUNDARY, bdrLen);

  StreamActive = true;
  StreamingLoop();
}

const char JHEADER[] = "HTTP/1.1 200 OK\r\n" \
                       "Content-disposition: inline; filename=capture.jpg\r\n" \
                       "Content-type: image/jpeg\r\n\r\n";
const int jhdLen = strlen(JHEADER);

void handle_jpg(void) {
  PrintMessageLn(F("Handle Jpg"));
  fb = esp_camera_fb_get();
  WiFiClient client = webServer.client();
  client.write(JHEADER, jhdLen);
  client.write((char *)fb->buf, fb->len);
}

void StreamingLoop() {
  if (!StreamActive)
    return;
  if (!streamingClient.connected()) {
    StreamActive = false;
    return;
  }
  char buf[32];
  fb = esp_camera_fb_get();
  int s = fb->len;
  streamingClient.write(CTNTTYPE, cntLen);
  sprintf( buf, "%d\r\n\r\n", s );
  streamingClient.write(buf, strlen(buf));
  streamingClient.write((char *)fb->buf, s);
  streamingClient.write(BOUNDARY, bdrLen);
}
