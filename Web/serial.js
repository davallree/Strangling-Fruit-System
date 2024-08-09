class LineBreakTransformer {
  chunks = "";

  transform(chunk, controller) {
    this.chunks += chunk;
    const lines = this.chunks.split("\n");
    this.chunks = lines.pop();
    lines.forEach((line) => controller.enqueue(line));
  }
}

class JsonTransformer {
  transform(chunk, controller) {
    try {
      controller.enqueue(JSON.parse(chunk));
    } catch (e) {
      console.warn("Could not parse chunk as JSON, discarding: ", chunk);
    }
  }
}

export class SerialHandler {
  messageCallback = null;

  async connect() {
    this.port = await navigator.serial.requestPort();
    await this.port.open({ baudRate: 115200 });  // Ensure baud rate matches the ESP32 settings

    const decoder = new TextDecoderStream();
    this.port.readable.pipeTo(decoder.writable);
    this.inputStream = decoder.readable
      .pipeThrough(new TransformStream(new LineBreakTransformer()))
      .pipeThrough(new TransformStream(new JsonTransformer()));
    this.reader = this.inputStream.getReader();

    this.encoder = new TextEncoderStream();
    this.encoder.readable.pipeTo(this.port.writable);
    this.writer = this.encoder.writable.getWriter();
  }

  async read() {
    while (true) {
      try {
        const { value, done } = await this.reader.read();
        if (done) {
          console.log('[readLoop] DONE', done);
          this.reader.releaseLock();
          break;
        }
        if (value && this.messageCallback) {
          this.messageCallback(value);
        }
      } catch (error) {
        console.error('Error reading data: ', error);
        break;
      }
    }
  }

  async send(method, params) {
    const message = JSON.stringify({ method, params });
    console.log('Sending message: ', message);
    await this.writer.write(message);
  }
}
