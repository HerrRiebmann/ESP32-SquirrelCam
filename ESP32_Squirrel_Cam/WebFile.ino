const char HTML_HEADER[] =
  "<!DOCTYPE HTML>"
  "<html lang='en'>"
  "<head>"
  "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
  "<title>File-List</title>"
  "<link rel='stylesheet' type='text/css' href='style.css' />"
  "</head>"
  "<body><div style='margin-left:30px;'>";
const char HTML_TABLE[] =
  "<table class='greyGridTable'><thead><tr><th width='80%'>File/Dir</th><th width='180px'>Size</th><th width='70px'>Delete</th></tr></thead><tbody>";
const char HTML_END[] =
  "</tbody></table>"
  "</div></body>"
  "</html>";

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html") || filename.endsWith(".htm"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css; charset=utf-8";
  else if (filename.endsWith(".js"))
    return "application/javascript; charset=utf-8";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".jpg"))
    return "image/jpg";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".xml"))
    return "text/xml";
  else if (filename.endsWith(".pdf"))
    return "application/x-pdf";
  else if (filename.endsWith(".zip"))
    return "application/x-zip";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  return "text/plain";
}

void handleFileRead() {
  String path = webServer.uri();
  PrintMessageLn("Handle FileRead: " + path);

  //Stay alive, when Webserver is used:
  if (!skipDeepsleep) {
    PrintMessageLn("Skip Deepsleep due to webserver!");
    skipDeepsleep = true;
  }

  if (IsFileList(path))
    return;

  // If a folder is requested, send index.html
  if (path.endsWith("/"))
    path.concat("index.html");

  // If request is captive request, followed with a GUID
  if (path.startsWith("/generate_204")) {
    redirect();
    return;
  }
  if (GetFS().exists(path)) {
    File file = GetFS().open(path, "r");
    if (file) {
      String fileSize = String(file.size());
      //Check File "Version" (Size) is still the same, otherwise sumbit it
      if (ProcessETag(fileSize.c_str())) {
        file.close();
        return;
      }
      size_t sent = webServer.streamFile(file, getContentType(path));
      file.close();
      PrintMessage(String("\tSent file: ") + path);
      PrintMessageLn(" " + String(sent));
      return;
    }
  }

  handleNotFound();
  PrintMessageLn(String("\tFile Not Found: ") + path);
}

bool IsFileList(String& path) {
  String directory;
  String filename;
  if (webServer.hasArg("DIR"))
    directory = webServer.arg("DIR");
  if (webServer.hasArg("FN"))
    filename = webServer.arg("FN");

  //PrintMessage("Directory "); PrintMessage(directory); PrintMessage(" File: "); PrintMessageLn(filename);
  if (webServer.hasArg("DEL")) {
    PrintMessageLn("Delete");
    if (GetFS().exists(filename)) {
      PrintMessageLn(F("\tFile Deleted"));
      GetFS().remove(filename);
    }
    //Do not load all files again:
    if (webServer.hasArg("SKIP")) {
      webServer.send(200,  "text/html", "OK");
      return true;
    }
    if (directory.length() == 0)
      sendDirectory("/");
    else
      sendDirectory(directory);
    return true;
  }
  if (path.endsWith("files.html") && (directory.length() == 0 && filename.length() == 0)) {
    PrintMessageLn("Root-Directory");
    sendDirectory("/");
    return true;
  }
  if (directory.length() > 0 && filename.length() == 0) {
    PrintMessageLn("Directory");
    sendDirectory(directory);
    return true;
  }
  if (filename.length() > 1) {
    PrintMessageLn("Filename");
    path = filename;
  }
  return false;
}

bool sendDirectory(String path) {
  PrintMessageLn("Send Dir");
  String subdir;
  String parent;
  String name;

  if (!FSinitialized) {
    String fsFailed = "<h2 style='color: red;'>" + GetFileSystemName() + " not initialized!</h2>";
    webServer.send(200,  "text/html", fsFailed);
    return false;
  }

  File dirFile = GetFS().open(path, FILE_READ);
  if (dirFile) {
    PrintMessageLn(dirFile.name());
    webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webServer.send(200,  "text/html", HTML_HEADER);
    webServer.sendContent("<h2>" + GetFileSystemName() + "</h2>");
    webServer.sendContent(HTML_TABLE);
    WiFiClient client = webServer.client();
    if (path != "/") {
      //Add ".." to navigate back to root
      parent = path;
      parent.remove(parent.lastIndexOf("/"));
      webServer.sendContent("<tr><td>");
      webServer.sendContent("<a href='?DIR=");
      webServer.sendContent(parent);
      if (path.startsWith("//"))
        webServer.sendContent("'>" + path.substring(2) + "/..</a>");
      else
        webServer.sendContent("'>" + path + "/..</a>");
      webServer.sendContent("</td><td></td><td></td></tr>");

    }
    File file = dirFile.openNextFile();
    while (file) {
      //Filename lesen
      name = file.name();
      PrintMessage("\t");
      PrintMessageLn(name);
      webServer.sendContent("<tr><td>");
      webServer.sendContent("<a href='?DIR=");
      webServer.sendContent(path);

      if (file.isDirectory()) {
        //directory
        subdir = name;
        webServer.sendContent(subdir);
        webServer.sendContent("'>");
      } else {
        //file
        webServer.sendContent("&FN=");
        webServer.sendContent(name);
        webServer.sendContent("' target='_blank'>");
      }
      //File/Folder-name
      if (path != "/") {
        String shortName = name;
        shortName.replace(path, "");
        webServer.sendContent(shortName);
      }
      else
        webServer.sendContent(name);
      //End link
      webServer.sendContent("</a>");
      webServer.sendContent("</td><td>");
      if (!file.isDirectory()) {
        webServer.sendContent(formatBytes(file.size()));
        webServer.sendContent("</td><td><a href='?FN=");
        webServer.sendContent(name);
        webServer.sendContent("&DEL=1&DIR=" + path + "' target=''><font color='red'>X</font></a></td></tr>");
      }
      else
        webServer.sendContent("</td><td></td></tr>");
      file = dirFile.openNextFile();
    }
    file.close();
    dirFile.close();
    webServer.sendContent(HTML_END);
    client.stop();
    return true;
  } else {
    PrintMessageLn("Open directory failed!");
    return false;
  }
}

bool sendImages() {
  PrintMessageLn("Send Images");
  String path = "/Photos";
  String subdir;
  String parent;
  String name;

  File dirFile = GetFS().open(path, FILE_READ);
  if (dirFile) {
    PrintMessageLn(dirFile.name());
    webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webServer.send(200,  "text/plain", "");
    WiFiClient client = webServer.client();

    File file = dirFile.openNextFile();
    while (file) {
      //Filename lesen
      name = file.name();
      PrintMessage("\t");
      PrintMessageLn(name);
      if (!file.isDirectory())
        webServer.sendContent(name);
      file = dirFile.openNextFile();
      if (file)
        webServer.sendContent("|");
    }
    file.close();
    dirFile.close();
    client.stop();
    return true;
  } else {
    PrintMessageLn("Open directory failed!");
    return false;
  }
}

const String formatBytes(size_t const& bytes) {
  return " (" + (bytes < 1024 ? static_cast<String>(bytes) + " Byte" : bytes < 1048576 ? static_cast<String>(bytes / 1024.0) + " KB" : static_cast<String>(bytes / 1048576.0) + " MB") + ")";
}

File fsUploadFile;
void handle_fileupload(HTTPUpload& upload) {
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;

    if (filename.indexOf("index.html") <= 0 && (filename.endsWith(".html") || filename.endsWith(".js") || filename.endsWith(".css"))) {
      filename = "/web" + filename;
      PrintMessageLn("Send file to subfolder 'web'");
    }

    PrintMessage("handleFileUpload: ");
    PrintMessageLn(filename);

    if (GetFS().exists(filename)) {
      PrintMessageLn(F("\tFile Deleted"));
      GetFS().remove(filename);
    }

    fsUploadFile = GetFS().open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
      PrintMessage("handleFileUpload Size: ");
      PrintMessageLn(String(upload.totalSize));
      // Redirect the client to the root page
      webServer.sendHeader("Location", "/");
      webServer.send(303);
    } else {
      webServer.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

bool ProcessETag(const char* ETag) {
  for (int i = 0; i < webServer.headers(); i++) {
    if (webServer.headerName(i).compareTo(F("If-None-Match")) == 0)
      if (webServer.header(i).compareTo(ETag) == 0) {
        webServer.sendHeader("Cache-Control", "max-age=31536000, immutable");
        webServer.send(304, "text/plain", F("Not Modified"));
        PrintMessageLn(String(F("\t")) + webServer.headerName(i) + F(": ") + webServer.header(i));
        return true;
      }
  }
  webServer.sendHeader("ETag", ETag);
  webServer.sendHeader("Cache-Control", "max-age=31536000, immutable");
  return false;
}
