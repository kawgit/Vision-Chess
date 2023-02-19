importScripts("Vision.js");

let wrapped_set_position = Module.cwrap("set_position", null, ["string","string"]);

let wrapped_set_num_threads = Module.cwrap("set_num_threads", null, ["number"]);

let wrapped_set_max_time = Module.cwrap("set_max_time", null, ["number"]);

let wrapped_launch = Module.cwrap("launch", null, []);

let wrapped_worker = Module.cwrap("worker", null, ["number"]);

let wrapped_stop = Module.cwrap("stop", null, []);

let wrapped_probe_move = Module.cwrap("probe_move", "string", []);

let wrapped_probe_eval = Module.cwrap("probe_eval", "number", []);

let wrapped_probe_depth = Module.cwrap("probe_depth", "number", []);

let wrapped_probe_pv = Module.cwrap("probe_pv", "string", []);