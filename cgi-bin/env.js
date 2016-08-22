const response = JSON.stringify(process.env);
console.log(`Content-Type: text/plain
Content-Length: ${response.length}

${response}`);
