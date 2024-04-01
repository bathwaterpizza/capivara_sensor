#include "webpage.hpp"

// This is the configuration webpage that will be available at our AP at port 80
const char webpage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
  <head>
    <title>Capivara</title>
    <style>
      body,
      html {
        height: 100%;
        margin: 0;
        display: flex;
        justify-content: center;
        align-items: center;
        background-color: navy;
      }

      .container {
        text-align: center;
      }

      .centered-form {
        display: inline-block;
      }

      h1,
      label {
        color: white;
        font-size: 3vw;
        max-width: 100%;
      }

      input[type="submit"] {
        font-size: 3vw;
        max-width: 100%;
      }

      input[type="text"] {
        font-size: 3vw;
        max-width: 100%;
        margin: 1vw;
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>Device Settings</h1>
      <br /><br />

      <form action="/setcfg" method="POST" class="centered-form">
        <input type="radio" id="i1" name="wifi" value="0" required />
        <label for="i1">DI-GRAD</label><br /><br />

        <input type="radio" id="i2" name="wifi" value="1" required />
        <label for="i2">Wi-Fi PUC</label><br /><br />

        <input type="radio" id="i3" name="wifi" value="2" required />
        <label for="i3">LIENG</label><br /><br />

        <input type="radio" id="i4" name="wifi" value="3" required />
        <label for="i4">Redmi 9A</label><br /><br />

        <br /><br />

        <label for="classroomid">CLASSROOM_ID</label><br />
        <input type="text" id="classroomid" name="classroom_id" /><br /><br /><br />

        <br /><input type="submit" value="Save and reboot" />
      </form>
    </div>
  </body>
</html>
)html";