/**
 * Example for the ESP32 HTTP(S) Webserver
 *
 * IMPORTANT NOTE:
 * To run this script, you need to
 *  1) Enter your WiFi SSID and PSK below this comment
 *
 * This script will install an HTTPS Server on your ESP32 with the following
 * functionalities:
 *  - Show simple page on web server root
 *  - 404 for everything else
 * 
 * In contrast to the other examples, the certificate and the private key will be
 * generated on the ESP32, so you do not need to provide them here.
 * (this means no need to run create_cert.sh)
 */

// TODO: Configure your WiFi here
#define WIFI_SSID "NETGEAR82_EXT"
#define WIFI_PSK  "wittybutter395"

// We will use wifi
#include <WiFi.h>

#include "cert.h"
#include "private_key.h"

// Includes for the server
#include "HTTPSServer.hpp"
#include "SSLCert.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

SSLCert * cert;
HTTPSServer * secureServer;
uint64_t requestsHandled = 0;

// Declare some handler functions for the various URLs on the server
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);

void setup() {
  // For logging
  Serial.begin(115200);
  delay(3000); // wait for the monitor to reconnect after uploading.

  //Serial.println("Creating a new self-signed certificate.");
  //Serial.println("This may take up to a minute, so be patient ;-)");

  // First, we create an empty certificate:
  //cert = new SSLCert();

  cert = new SSLCert(
    example_crt_DER, example_crt_DER_len,
    example_key_DER, example_key_DER_len
  );

  // Now, we use the function createSelfSignedCert to create private key and certificate.
  // The function takes the following paramters:
  // - Key size: 1024 or 2048 bit should be fine here, 4096 on the ESP might be "paranoid mode"
  //   (in generel: shorter key = faster but less secure)
  // - Distinguished name: The name of the host as used in certificates.
  //   If you want to run your own DNS, the part after CN (Common Name) should match the DNS
  //   entry pointing to your ESP32. You can try to insert an IP there, but that's not really good style.
  // - Dates for certificate validity (optional, default is 2019-2029, both included)
  //   Format is YYYYMMDDhhmmss
  //int createCertResult = createSelfSignedCert(
  //  *cert,
  //  KEYSIZE_1024,//KEYSIZE_2048,
  //  "CN=myesp32.local,O=FancyCompany,C=DE",
  //  "20190101000000",
  //  "20300101000000"
  //);

  // Now check if creating that worked
  //if (createCertResult != 0) {
  //  Serial.printf("Cerating certificate failed. Error Code = 0x%02X, check SSLCert.hpp for details", createCertResult);
  //  while(true) delay(500);
  //}
  //Serial.println("Creating the certificate was successful");

  // If you're working on a serious project, this would be a good place to initialize some form of non-volatile storage
  // and to put the certificate and the key there. This has the advantage that the certificate stays the same after a reboot
  // so your client still trusts your server, additionally you increase the speed-up of your application.
  // Some browsers like Firefox might even reject the second run for the same issuer name (the distinguished name defined above).
  //
  // Storing:
  //   For the key:
  //     cert->getPKLength() will return the length of the private key in byte
  //     cert->getPKData() will return the actual private key (in DER-format, if that matters to you)
  //   For the certificate:
  //     cert->getCertLength() and ->getCertData() do the same for the actual certificate data.
  // Restoring:
  //   When your applications boots, check your non-volatile storage for an existing certificate, and if you find one
  //   use the parameterized SSLCert constructor to re-create the certificate and pass it to the HTTPSServer.
  //
  // A short reminder on key security: If you're working on something professional, be aware that the storage of the ESP32 is
  // not encrypted in any way. This means that if you just write it to the flash storage, it is easy to extract it if someone
  // gets a hand on your hardware. You should decide if that's a relevant risk for you and apply countermeasures like flash
  // encryption if neccessary

  // We can now use the new certificate to setup our server as usual.
  secureServer = new HTTPSServer(cert);

  // Connect to WiFi
  Serial.println("Setting up WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected. IP=");
  Serial.println(WiFi.localIP());

  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode * nodeRoot    = new ResourceNode("gemini://sdfgeoff.ddns.net/", "GET", &handleRoot);
  ResourceNode * nodeRoot2    = new ResourceNode("gemini://sdfgeoff.ddns.net", "GET", &handleRoot);
  ResourceNode * nodeDetails    = new ResourceNode("gemini://sdfgeoff.ddns.net/details", "GET", &handleDetailsPage);
  

  // Add the root node to the server
  secureServer->registerNode(nodeRoot);
  secureServer->registerNode(nodeRoot2);
  secureServer->registerNode(nodeDetails);
  // Add the 404 not found node to the server.
  
  ResourceNode * node404     = new ResourceNode("", "GET", &handle404);
  secureServer->setDefaultNode(node404);

  Serial.println("Starting server...");
  secureServer->start();
  if (secureServer->isRunning()) {
    Serial.println("Server ready.");
  }
}

void loop() {
  // This call will let the server do its work
  secureServer->loop();

  // Other code would go here...
  delay(1);
}

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  res->setStatusText("text/gemini");  
  res->println("# The First Gemini Page");
  res->println("This is running from an ESP32 Microcontroller using a hacked HTTPS Server library. So, uh, sorry if it isn't fully compliant");
  res->println("=> gemini://sdfgeoff.ddns.net/details Implementation Details");
  
  res->println("## Server Stats:");
  res->print("Uptime: ");
  res->print(res->print((int)(millis()/1000), DEC));
  res->println(" seconds");
  res->print("Requests Served: ");
  res->println((int)requestsHandled, DEC);
  requestsHandled += 1;
}

void handleDetailsPage(HTTPRequest * req, HTTPResponse * res) {
  
  res->setStatusText("text/gemini");



res->println("#  Motivation\n");
res->println("For a while now I\'ve been drifting around on Gemini - mostly on station.martinrue.com. I\'ve been unable to host my own capsule because, well, I\'m lazy.\n\n* I can\'t be bothered joining a tilde club\n* I can\'t be bothered buying a wifi adapter for my raspberry Pi 2\'s\n* I can\'t be bothered routing an ethernet cable to my room\n* I can\'t be bothered buying a second hand PC that I\'ll have to put somewhere\n\nBut sitting on my desk is an ESP32 microcontroller with wifi built in. It\'s 240Mhz dual core, and in theory has enough power to host a capsule. Heck, people connect cameras to this thing(an ESP32 camera module = <$10 from aliexpress).\n\nSo now the problem is that there isn\'t any existing server softwar for it. But that\'s OK, there\'s an arduino library that sets up a HTTPS server. The TLS is the \"complex stuff I  can\'t be bothered learning\" so if I can just hack around with that server library, I should be good to go.\n\n");
res->println("#  Resources\n\n=> https://github.com/fhessel/esp32_https_server HTTPS Library\n=> https://gemini.circumlunar.space/docs/specification.gmi Gemini Spec\n=> https://sr.ht/~acdw/bollux/ Bollux - a bash gemini browser\n\n");
res->println("#  Session 1 Progress\n\n");
res->println("##  Testing it out\nSo I load the esp32_https_server example code onto the esp32 and browse to it with firefox. Cool! It works! Now lets browse to it with lagrange. Aww, it doesn\'t work.\n\n");
res->println("##  Changing the port\nGemini is on port 1965, so lets fix that first. That\'s defined in HTTPSServer.cpp. Now lagrange is reporting non-useful errors, so I\'ll switch to bollux which apparently gives nice error messages.\n\n");
res->println("##  Parsing the URL\nbollux reports:\n```\n./bollux: line 518: HTTP/1.1: syntax error: invalid arithmetic operator (error token is \".1\")\n```\nLooks like the first part of an HTTP header is converting to math inside bollux?! That\'s probably a security issue in bollux. What if my header was \"$rm -rf ~/*\"? Oh well, not my issue at the moment.\n\nThe server is also logging an error:\n```\n[HTTPS:W] Missing space after method\n```\nThat\'s something we definitely can fix. We need to teach our http server how to parse gemini headers. Grepping for \"Missing space after method\" brings us to HTTPConnection.cpp which seems like a good place to start. Looks like there\'s a state machine there that handles parsing. We really don\'t need to parse our gemini request. A gemini request is always a \"GET\" like thing, and the whole header is the URL. So we can tear out all the conditionals and error checking and replace things like:\n```\nsize_t spaceAfterResourceIdx = _parserLine.text.find(\'\\r\\n\');\nif (spaceAfterResourceIdx == std::string::npos) {\n  HTTPS_LOGW(\"Missing space after resource\");\n  raiseError(400, \"Bad Request\");\n  break;\n}\n_httpResource = _parserLine.text.substr(spaceAfterMethodIdx + 1, spaceAfterResourceIdx - _httpMethod.length() - 1);\n```\nwith:\n```\n_httpMethod = \"GET\"\n_httpResource = _parserLine.text;\n```\nHah. Gotta love how simple gemini protocol is.\n\nOur server is now returning a lovely:\n```\n[HTTPS:I] Request: GET gemini://myesp32.local (FID=55)\n```\nand forwarding it on to the rest of the server stack. (Don\'t ask me what goes on in there!)\n\n");
res->println("##  Why u timeout?\nNow everything is timing out! I guess the server is waiting for the HTTP body from the client. So lets look at the state machine a bit more. It normally goes from\n```\nSTATE_INITIAL -> STATE_REQUEST_FINISHED -> STATE_HEADERS_FINISHED -> STATE_BODY_FINISHED -> ????\n```\nSo yeah, I guess it\'s waiting for a body from the client. That\'s fine, we can just change the end state after parsing the headers:\n```\n//_connectionState = STATE_REQUEST_FINISHED;\n_connectionState = STATE_HEADERS_FINISHED;\n```\nSweet. No more timeouts.\n\n");
res->println("##  Where barf now?\nBollux is now doing a bit better:\n```\nbollux:die:\tTemporary error [400]: Bad Request\n```\nAt least it\'s now hitting a known gemini thing - even if it is a \"bad request\". Turns out that the first two characters of a HTTP 404 response happen to coincide with a gemini temporary error. Who knew!\n\nHTTP urls are specified by relative path, gemini includes the URL. Let\'s ignore this problem for now and give it a proper resource.\n```\n//ResourceNode * nodeRoot    = new ResourceNode(\"/\", \"GET\", &handleRoot);\nResourceNode * nodeRoot    = new ResourceNode(\"gemini://myesp32.local/\", \"GET\", &handleRoot);\n```\nSweet!\n```\nbollux:die:\tEmpty response code.\n```\nDarn. You can do it bollux! Live!\n\nLet\'s go poke at HTTPResponse.cpp - it seems like it would be relevant.\n```\n// Status line, like: \"HTTP/1.1 200 OK\\r\\n\"\n//std::string statusLine = \"HTTP/1.1 \" + intToString(_statusCode) + \" \" + _statusText + \"\\r\\n\";\n//printInternal(statusLine, true);\n\n// Each header, like: \"Host: myEsp32\\r\\n\"\n//std::vector<HTTPHeader *> * headers = _headers.getAll();\n//for(std::vector<HTTPHeader*>::iterator header = headers->begin(); header != headers->end(); ++header) {\n//  printInternal((*header)->print()+\"\\r\\n\", true);\n//}\n//printInternal(\"\\r\\n\", true);\n\n\n// Status line, like IDK!\nstd::string statusLine = intToString(_statusCode) + \" \" + _statusText + \"\\r\\n\";\nprintInternal(statusLine, true);\n```\nBegone all you header guff.\n\n");
res->println("##  Download the page eh?\n```\nbollux:download:\tDownloading: \'gemini://myesp32.local\' => \'/tmp/tmp.eFVo9VJj5o\'...\n<snip>\nbollux:download:\tSaved \'./myesp32.local\'.\n```\nHmm, Ok. ./myesp32.local contains a bunch of HTML that I\'m serving (hey, I haven\'t got there yet). Let\'s try in lagrange.\n\n```\nUnknown Status Code\nThe server responded with a status code that is not in the Gemini protocol specification. Maybe the server is from the future? Or just malfunctioning.\nServer responded with the message:\n\n\"0 OK\"\n```\n\nDarn. I guess my server is returning \"200 OK\" currently. Bollux is looking at the first two characters of the response and ignoring the rest. I\'ve no idea how lagrange decides to split/display to get that message? Perhaps it detects the response code as 200, but it\'s just the displaying that removes the first two characters. Either way it\'s an easy fix.\n```\n_statusCode = 20; // 200;\n```\n\nNow lagrange gives:\n```\nUnsupported Content Type\nThe received content cannot be viewed with this application.\n\nOK\n```\n(The OK is the content type)\nSweet! We\'re now serving valid gemini content! We\'re serving an HTML page with the mime type \"OK\" but ...details...\n\n");
res->println("##  End Session 1\nThis is definitely possible. I reckon another hour or two of work and it\'ll be good to go. Time to\ngo hang up some washing, buy some vegetables and get on with my Saturday morning (ahem, afternoon)\n\n");
res->println("###  Where am I going with this?\n\n* Serve static gemtext from the ESP\'s flash\n* Serve a dynamic page from the server (uptime, status etc.)\n* Serve pages from a SD card.\n\nMy current plan is to load content onto my server by ... switching SD cards out! Maybe in future I\'ll\ngive it some other interface?\n\nAlso, maybe I could make this server solar powered. An ESP32 draws ~1W when active (300mA, 3.3v), so\nI need maybe ~20Wh of battery and 10W of panel. That\'s a moderate USB battery bank and a pretty small\npanel.\n\n");
res->println("#  Session 2 progress\nRighto, where were we. Ahh, we need to consider the response a bit more. A response has three parts:\n* Status Code\n* META field (data about the status code)\n* Body.\nParts 1 and 2 are in the header. Our http response object a status code and a status text field, so let\'s just pretend that our status text field is the META field.\n\n");
res->println("##  Handling 40(4\'s)\nLet\'s just poke around some more. Let\'s use lagrange to go to a non-existant page:\n```\nThe server responded with a status code that is not in the Gemini protocol specification. Maybe the server is from the future? Or just malfunctioning.\n\nServer responded with the message:\n4 Not Found\n```\n\nFortunately our example code assigns a specific function to the 404 page:\n```\nResourceNode * node404     = new ResourceNode(\"\", \"GET\", &handle404);\nsecureServer->setDefaultNode(node404);\n```\nWe can easily tweak that function to give a gemini response:\n```\nvoid handle404(HTTPRequest * req, HTTPResponse * res) {\n  // Discard request body, if we received any\n  // We do this, as this is the default node and may also server POST/PUT requests\n  req->discardRequestBody();\n\n  // Set the response status\n  //res->setStatusCode(404);\n  res->setStatusCode(40);\n  res->setStatusText(\"Not Found\");\n\n  // Set content type of the response\n  //res->setHeader(\"Content-Type\", \"text/html\");\n\n  // Write a tiny HTTP pag3\n  //res->println(\"<!DOCTYPE html>\");\n  //res->println(\"<html>\");\n  //res->println(\"<head><title>Not Found</title></head>\");\n  //res->println(\"<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>\");\n  //res->println(\"</html>\");\n}\n```\nBut now things are timing out again. I guess the server is keeping the connection open? By re-enabling one of the res->println\'s the timeouts go away - but then it isn\'t gemini compliant anymore :(. So it seems that the server assumes all responses have bodies.\n\nMaybe I broke something earlier. Way back in \"Why u timeout\", we changed\n```\n//_connectionState = STATE_REQUEST_FINISHED;\n_connectionState = STATE_HEADERS_FINISHED;\n```\nLets see what\'s happening in here to cause those timeouts originally. Turns out that the function there has this wonderful structure:\n```\ncase STATE_REQUEST_FINISHED:\n  while(_bufferProcessed < _bufferUnusedIdx && !isClosed()) {\n    <do useful stuff>\n  }\n  break;\ncase STATE_HEADERS_FINISHED: // Handle body\n```\nInside the <do useful stuff> it transitions state when all the body has been sent. In our case we have no body to send but never get to the state transition! In this case the <do useful stuff> is to parse the HTTP headers, so we can continue bypassing this state. I guess this means that this server will fail if there is ever a HTTP request with no headers. Good thing no browser does this :p\n\n*RANT: Name your states with what they do not with what the previous one has finished doing!*\n```\n  // The connection has not been established yet\n  STATE_UNDEFINED, -- STATE_PASS\n\n  // The connection has just been created\n  STATE_INITIAL, -- STATE_READ_REQUEST_TYPE_AND_URL\n\n  // The request line has been parsed\n  STATE_REQUEST_FINISHED, -- STATE_PARSE_REQUEST_HEADERS\n\n  // The headers have been parsed\n  STATE_HEADERS_FINISHED, -- STATE_GENERATE_RESPONSE_AND_DO_EVERYTHING_ELSE\n\n  // The body has been parsed/the complete request has been processed (GET has body of length 0)\n  STATE_BODY_FINISHED, -- STATE_BEGIN_CLOSE_THE_CONNECTION\n\n  // The connection is in websocket mode\n  STATE_WEBSOCKET, -- STATE_HANDLE_WEBSOCKET_I_GUESS\n\n  // The connection is about to close (and waiting for the client to send close notify)\n  STATE_CLOSING, -- STATE_WAIT_FOR_CONNECTION_TO_CLOSE\n\n  // The connection has been closed\n  STATE_CLOSED,\n\n  // An error has occured\n  STATE_ERROR\n```\nClearly we need to investigate the STATE_HEADERS_FINISHED some more. After digging for an hour or two I decided I was overthinking it. If it worked with sending a body, then I could \"solve\" it by sending an empty body:\n```\nvoid handle404(HTTPRequest * req, HTTPResponse * res) {\n  req->discardRequestBody();\n\n  res->setStatusCode(40);\n  res->setStatusText(\"Not Found\");\n  res->println(\"\");\n}\n```\nNo idea if this is spec-compliant or not. It may well send an extra \"\\r\\n\" at the end, but it works in both lagrange and bollux, so whatever.\n\n");
res->println("##  Serving actual data\nIn theory we should be good to go\n```\nvoid handleRoot(HTTPRequest * req, HTTPResponse * res) {\n  res->setStatusText(\"text/gemini\");  \n  res->println(\"#  The First Gemini Page\");\n  res->println(\"This is running from an ESP32 Microcontroller using a hacked HTTP Server library. So, uh, sorry if it isn\'t fully compliant\");\n}\n```\nAnd, yup. Works in both Lagrange and bollux. \n\"");

  requestsHandled += 1;
  
  
}


void handle404(HTTPRequest * req, HTTPResponse * res) {
  // Discard request body, if we received any
  // We do this, as this is the default node and may also server POST/PUT requests
  req->discardRequestBody();

  // Set the response status
  //res->setStatusCode(404);
  res->setStatusCode(40);
  res->setStatusText("Not Found");
  res->println("");

  // Set content type of the response
  //res->setHeader("Content-Type", "text/html");

  // Write a tiny HTTP pag3
  //res->println("<!DOCTYPE html>");
  //res->println("<html>");
  //res->println("<head><title>Not Found</title></head>");
  //res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  //res->println("</html>");
}
