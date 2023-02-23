const fs = require("fs");
const child_process = require("child_process");

let project = new Project('mackrafixrepro');

project.addFile('Sources/**');
project.addIncludeDir('Sources');
let krafix = await project.addProject('krafix');
krafix.useAsLibrary();
await project.addProject('Kinc');

project.setDebugDir('Deployment');
//fs.copyFileSync('shadergen/Shaders/sdfbase.frag.glsl', 'Deployment/sdfbase.frag.glsl');

project.flatten();
resolve(project);
