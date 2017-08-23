function getValue(name) { return document.getElementsByName(name)[0].value }
function setStrValue(name, val) { document.getElementsByName(name)[0].value = val }

if (!('WebAssembly' in window)) {
	alert('WebAssembly is not supported')
}

function loadWebAssembly(filename, imports) {
	return fetch(filename)
	.then(response => response.arrayBuffer())
	.then(buffer => WebAssembly.compile(buffer))
	.then(module => {
		imports = imports || {}
		imports.env = imports.env || {}
		imports.env.memoryBase = imports.env.memoryBase || 0
		imports.env.tableBase = imports.env.tableBase || 0
		if (!imports.env.memory) {
			imports.env.memory = new WebAssembly.Memory({ initial: 256 })
		}
		if (!imports.env.table) {
			imports.env.table = new WebAssembly.Table({ initial: 0, element: 'anyfunc' })
		}
		return new WebAssembly.Instance(module, imports)
	})
}

