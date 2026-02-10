class TDiagram {

	file_ext(file)
	{
	    var name = file.name
	    if (name == "/kaem.x86")
	        return "kaem"
	    if (name.indexOf("/M2-Mesoplanet-000000") > 0)
	        return "c"
	    if (name.indexOf("/M2-Planet-000000") > 0 || name.indexOf("/blood-elf-000000") > 0)
	        return "M1"
	    if (name.indexOf("/M1-macro-000000") > 0)
	        return "hex2"
	    if (name.indexOf("/tcc-boot0") > 0)
	        return "elf"
	    var last_pt = name.lastIndexOf(".")
	    if (last_pt == -1)
	        return ""
	    var last_sl = name.lastIndexOf("/")
	    if (last_sl > last_pt)
	        return ""
	    return name.substring(last_pt + 1)
	}
	file_name(name)
	{
	    var last_sl = name.lastIndexOf("/")
	    return last_sl == -1 ? name : name.substring(last_sl + 1)
	}
	
	file_ref(action, by, proc)
	{
	    var name = this.data.files[action.file].name
	    var label = this.file_name(name)
	    var w = this.ctx.measureText(label).width
	    var out = null
	    var file = this.data.files[action.file]
	    file.w = w
	    if (by != undefined)
	    {
	        var by_proc = this.data.processes[by - 1]
	        //console.log("Find output of " + name + " as output of " + by_proc.nr + " outs " + by_proc.outs.length)
	        for (var i = 0; i < by_proc.outs.length; i++)
	        {
	            //console.log("Try " + by_proc.outs[i].file.name)
	            if (by_proc.outs[i].file.nr == file.nr)
	            {
	                //console.log("found")
	                out = by_proc.outs[i]
	                break
	            }
	        }
	    }
	    return { name:name, label:label, x:null, y:null, w:w, out:out, proc:proc, file:file }
	}
	
	used_as_input(file)
	{
	    for (var i = 0; i < file.actions.length; i++)
	        if (file.actions[i].kind == 'R')
	            return true
	    return false
	}
	
	has_child_processes(process)
	{
	    for (var a_i = 0; a_i < process.actions.length; a_i++)
	        if (process.actions[a_i].kind == 'E')
	            return true
	    return false
	}
	
	hide_process(process) { return process.outs.length == 0 && process.ins.length == 0 && !this.has_child_processes(process); }
	
	
	moveTo(x, y) { this.ctx.moveTo(this.dx + this.sf * x, this.dy + this.sf * y) }
	lineTo(x, y) { this.ctx.lineTo(this.dx + this.sf * x, this.dy + this.sf * y) }
	arcTo(x1, y1, x2, y2, r)  { this.ctx.arcTo(this.dx + this.sf * x1, this.dy + this.sf * y1, this.dx + this.sf * x2, this.dy + this.sf * y2, this.sf * r) }
	fillText(text, x, y) { this.ctx.fillText(text, this.dx + this.sf * x, this.dy + this.sf * y) }
	
	drawT(proc)
	{
	    if (proc.x == null)
	        return
	    var ewidth = this.elf_length / 2 + 2
	    var lwidth = proc.iw + ewidth
	    var rwidth = proc.ow + ewidth
	    var twidth = 30 + ewidth
	    var x = proc.x
	    var y = proc.y
	    this.ctx.beginPath()
	    this.moveTo(x - lwidth, y - 10)
	    this.lineTo(x + rwidth, y - 10)
	    this.lineTo(x + rwidth, y + 2)
	    this.lineTo(x + ewidth, y + 2)
	    this.lineTo(x + ewidth, y + 14)
	    this.lineTo(x - ewidth, y + 14)
	    this.lineTo(x - ewidth, y + 2)
	    this.lineTo(x - lwidth, y + 2)
	    this.ctx.closePath()
	    this.ctx.stroke()
	    this.fillText(proc.nr, x, y - 14)
	    this.fillText("elf", x, y + 12)
	    this.ctx.textAlign = "right"
	    this.fillText(proc.ie, x - ewidth, y)
	    this.ctx.textAlign = "left"
	    this.fillText(proc.oe, x + ewidth, y)
	    this.ctx.textAlign = "center"
	    var text_x = x + rwidth
	    for (var i = 0; i < proc.outs.length; i++)
	    {
	        var out = proc.outs[i]
	        if (out.x > 0)
	        {
	            this.ctx.beginPath()
	            this.moveTo(text_x, y - 3)
	            this.lineTo(out.x - out.w / 2 - 2, y - 3)
	            this.ctx.stroke()
	            text_x = out.x + out.w / 2 + 2
	            this.fillText(out.label, out.x, out.y)
	        }
	    }
	    if (proc.w != 0 && proc.h != 0)
	    {
	        //console.log(proc.w + " " + proc.h)
	        this.ctx.strokeStyle = "green" //"#E0E0E0"
	        this.ctx.beginPath()
	        this.moveTo(x - lwidth, y - 10)
	        this.lineTo(x - lwidth, y - 10 - proc.h)
	        this.lineTo(x - lwidth + proc.w, y - 10 - proc.h)
	        this.lineTo(x - lwidth + proc.w, y - 10)
	        this.lineTo(x + rwidth, y - 10)
	        this.ctx.stroke()
	        this.ctx.strokeStyle = "black"
	    }
	    if (proc.elf.out != null)
	    {
	        var out = proc.elf.out
	        this.ctx.beginPath()
	        if (out.x == x)
	            this.moveTo(out.x, out.y - 8)
	        else
	        {
	            this.moveTo(out.x + out.w/2 + 2, out.y - 3)
	            this.lineTo(x - 10, out.y - 3)
	            this.arcTo(x, out.y - 3, x, out.y - 13, 10)
	        }
	        this.lineTo(x, y + 14)
	        this.ctx.stroke()
	    }
	    else if (proc.elf.file != undefined)
	    {
	        this.ctx.beginPath()
	        this.moveTo(proc.elf.file.x, proc.elf.file.y - 9)
	        if (proc.elf.file.x < x)
	        {
	            this.arcTo(proc.elf.file.x, proc.elf.file.y - 19, proc.elf.file.x + 10, proc.elf.file.y - 19, 10)
	            this.lineTo(x - 10, proc.elf.file.y - 19)
	            this.arcTo(x, proc.elf.file.y - 19, x, proc.elf.file.y - 29, 10)
	        }
	        this.lineTo(x, y + 14)
	        this.ctx.stroke()
	    }
	    //var in_text_x = x - lwidth
	    for (var i = 0; i < proc.ins.length; i++)
	    {
	        var in_file = proc.ins[i]
	        if (in_file.out != null)
	        {
	            this.ctx.beginPath()
	            if (in_file.out.y == y)
	                this.moveTo(in_file.out.x + in_file.out.w / 2 + 2, in_file.out.y - 3)
	            else
	            {
	                this.moveTo(in_file.out.x, in_file.out.y - 10)
	                this.lineTo(in_file.out.x, y + 7)
	                this.arcTo(in_file.out.x, y - 3, in_file.out.x + 10, y - 3, 10)
	            }
	            this.lineTo(x - lwidth, y - 3)
	            this.ctx.stroke()
	            //if (in_file.out.x < in_text_x)
	            //    in_text_x = in_file.out.x
	        }
	    }
	    var prev_x = x - lwidth
	    for (var i = proc.ins.length - 1; i >= 0; i -= 1)
	    {
	        var in_file = proc.ins[i]
	        if (in_file.x != null)
	        {
	            this.ctx.beginPath()
	            this.moveTo(prev_x, y - 3)
	            this.lineTo(in_file.x + in_file.w / 2 + 2, y - 3)
	            this.ctx.stroke()
	            //in_text_x -= in_file.w / 2 + 2
	            this.fillText(in_file.label, in_file.x, y)
	            //in_file.x = in_text_x
	            //in_text_x -= in_file.w / 2 + 2
	            prev_x = in_file.x - in_file.w / 2 - 2
	        }
	    }
	    if (proc.tin != undefined)
	    {
	    //	drawT(this.ctx, prev_x, y, proc.tin[0], proc.tin[2], proc.tin[1])
	    	var fs = 1 / this.sf
	    	var ewidth1 = this.ctx.measureText(proc.tin[1]).width * fs / 2 + 2
		    var lwidth1 = this.ctx.measureText(proc.tin[0]).width * fs + 2 + ewidth1
		    var rwidth1 = this.ctx.measureText(proc.tin[2]).width * fs + 2 + ewidth1
			console.log(fs)
		    this.ctx.beginPath()
		    this.moveTo(prev_x, y - 3)
		    this.lineTo(prev_x - 20, y - 3)
		    this.ctx.stroke()
		    this.ctx.beginPath()
		    y -= 12
		    x = prev_x - 20 - ewidth1
		    this.moveTo(x - lwidth1, y - 10)
		    this.lineTo(x + rwidth1, y - 10)
		    this.lineTo(x + rwidth1, y + 2)
		    this.lineTo(x + ewidth1, y + 2)
		    this.lineTo(x + ewidth1, y + 14)
		    this.lineTo(x - ewidth1, y + 14)
		    this.lineTo(x - ewidth1, y + 2)
		    this.lineTo(x - lwidth1, y + 2)
		    this.ctx.closePath()
		    this.ctx.stroke()
		    this.fillText(proc.tin[1], x, y + 12)
		    this.ctx.textAlign = "right"
		    this.fillText(proc.tin[0], x - ewidth1, y)
		    this.ctx.textAlign = "left"
		    this.fillText(proc.tin[2], x + ewidth1, y)
		    this.ctx.textAlign = "center"

	    }
	    //this.ctx.strokeStyle = "black"
	}
	
	connectSeed(seed_file_label, elf_file_label)
	{
	    var seed_file = null
	    for (var i = 0; i < this.data.files.length; i++)
	        if (this.data.files[i].type == "seed" && this.data.files[i].label == seed_file_label)
	        {
	            seed_file = this.data.files[i]
	            break
	        }
	    var elf_file = null
	    for (var i = 0; i < this.data.processes.length; i++)
	        if (this.data.processes[i].outs.length == 1 && this.data.processes[i].outs[0].label == elf_file_label)
	        {
	            elf_file = this.data.processes[i].outs[0]
	            break
	        }
	    //if (seed_file == null) console.log("seed_file == null")
	    //if (elf_file == null) console.log("elf_file == null")
	    if (seed_file != null && elf_file != null)
	    {
	        this.ctx.strokeStyle = "red"
	        this.ctx.beginPath()
	        this.moveTo(elf_file.x, elf_file.y + 2)
	        this.lineTo(elf_file.x, seed_file.y - 13)
	        this.arcTo(elf_file.x, seed_file.y - 3, elf_file.x - 10, seed_file.y - 3, 10)
	        this.lineTo(seed_file.x + seed_file.w / 2 + 2, seed_file.y - 3)
	        this.ctx.stroke()
	    }
	}
	
	draw()
	{
	    this.ctx.fillStyle = "White"
	    this.ctx.beginPath()
	    this.ctx.rect(0, 0, this.can.width, this.can.height)
	    this.ctx.fill()
	    
	    this.ctx.lineWidth = this.sf
	    
	    this.ctx.font = Math.round(this.sf * 10) + "px serif";
	    this.ctx.strokeStyle = "black"
	    this.ctx.fillStyle = "black"
	    this.ctx.textAlign = "center"
	    
	    for (var i = 0; i < this.n; i++)
	        this.drawT(this.data.processes[i])
	    
	    this.ctx.textAlign = "center"
	    this.ctx.fillStyle = "red"
	    for (var i = 0; i < this.data.files.length; i++)
	    {
	        var file = this.data.files[i]
	        if (file.x > 0 || file.y > 0)
	            this.fillText(file.label, file.x, file.y)
	    }
	    
	    if (this.show_loops)
			for (var i = 0; i < this.data.processes.length; i++)
			{
				var process = this.data.processes[i];
				if (process.elf != null && process.elf.name == "/usr/bin/equal")
				{
					var file2 = process.ins[1]
					var p2 = null
					var out2 = null
					for (var j = 0; j < file2.file.actions.length; j++)
					{
						var action = file2.file.actions[j]
						if (action.kind == "W")
						{
							p2 = this.data.processes[action.proc-1]
							for (var k = 0; k < p2.outs.length; k++)
								if (p2.outs[k].file.name == file2.name)
								{
									out2 = p2.outs[k];
									break;
								}
							break;
						}
					}
					if (out2 != null)
					{
						var file1 = process.ins[0]
						for (var j = 0; j < file1.file.actions.length; j++)
						{
							var action = file1.file.actions[j]
							if (action.kind == "R" && action.proc != process.nr)
							{
								var p1 = this.data.processes[action.proc-1]
								for (var k = 0; k < p1.ins.length; k++)
									if (p1.ins[k].file.name == file1.name)
									{
										var in1 = p1.ins[k];
										this.ctx.strokeStyle = "#A0A0FF"
										this.ctx.beginPath()
										this.moveTo(in1.x, p1.y - 10)
										this.lineTo(in1.x, p2.y - 100)
										this.lineTo(out2.x, p2.y - 100)
										this.lineTo(out2.x, p2.y - 10)
										this.ctx.stroke()
										break;
									}
							}
						}
					}
				}
			}

	    // Connect two seeds
	    this.connectSeed("hex0-seed", "hex0")
	    this.connectSeed("kaem-optional-seed", "kaem-0")
	    //this.drawT(40, 0, "bin", "keam", "", 1150, 440)
	    //this.fillText("kaem-optional-seed", 40, 30)
	    //this.fillText("kaem.x86", -60, 0)
	}
	
	mouseup(event)
	{
	    console.log("Mouse up " + this.show_loops + " on " + this.can.id)
	    this.drag = false
	    var x = event.offsetX || (event.pageX - this.can.offsetLeft)
	    var y = event.offsetY || (event.pageY - this.can.offsetTop)
	    if (x != this.start_x || y != this.start_y)
	        return
	    //console.log("Click at "+x+", "+y + " dx: "+this.dx + " dy: "+this.dy + " sf: "+this.sf)
	    x = (x - this.dx) / this.sf
	    y = (y - this.dy) / this.sf
	    //console.log("Click at "+x+", "+y+" ("+(this.dx + this.sf * x)+ " "+(this.dy + this.sf * y)+ ")")
	    for (var i = 0; i < this.data.processes.length; i++)
	    {
	        var process = this.data.processes[i]
	        if (process.x != null && process.y >= y && y >= process.y - 12)
	        {
	            if (process.x - process.iw - this.elf_length / 2 - 2 <= x && x <= process.x + process.ow + this.elf_length / 2 + 2)
	            {
	                //console.log("Process "+process.nr + " y = "+process.y)
	                this.show_process_details(process)
	                return
	            }
	            for (var i2 = 0; i2 < process.ins.length; i2++)
	            {
	                var in_file = process.ins[i2]
	                if (in_file.out == null && in_file.x - in_file.w / 2 <= x && x <= in_file.x + in_file.w / 2)
	                {
	                    //console.log("Input file: "+in_file.file.label + " " + in_file.x + " " + in_file.w)
	                    this.show_file_details(in_file.file, process.nr)
	                    return
	                }
	            }
	            for (var i2 = 0; i2 < process.outs.length; i2++)
	            {
	                var out_file = process.outs[i2]
	                if (out_file.x - out_file.w / 2 <= x && x <= out_file.x + out_file.w / 2)
	                {
	                    //console.log("Ouput file: "+out_file.file.label)
	                    this.show_file_details(out_file.file, process.nr)
	                    return
	                }
	            }
	        }
	    }
	    for (var i = 0; i < this.data.files.length; i++)
	    {
	        var file = this.data.files[i]
	        if (   file.x != null && file.x - file.w / 2 <= x && x <= file.x + file.w / 2
	            && file.y >= y && y >= file.y - 12)
	        {
	            //show_file_details(file, 0)
	            return
	        }
	    }
	}
	
	mousedown(event, obj)
	{
	    document.body.style.mozUserSelect = document.body.style.webkitUserSelect = document.body.style.userSelect = 'none';
	    obj.last_x = event.offsetX || (event.pageX - obj.can.offsetLeft)
	    obj.last_y = event.offsetY || (event.pageY - obj.can.offsetTop)
	    obj.start_x = obj.last_x
	    obj.start_y = obj.last_y
	    obj.drag = true
	    console.log("Mouse down " + obj.show_loops + " on " + obj.can.id)
	}
	
	mousemove(event, obj)
	{
	    if (obj.drag)
	    {
	        var x = event.offsetX || (event.pageX - obj.can.offsetLeft)
	        var y = event.offsetY || (event.pageY - obj.can.offsetTop)
	        var d_x = x - obj.last_x
	        var d_y = y - obj.last_y
	        if (d_x != 0 || d_y != 0)
	        {
	            obj.dx += d_x
	            obj.dy += d_y
	            obj.last_x = x
	            obj.last_y = y
	            obj.draw()
	        }
		    //console.log("Drag " + obj,show_loops + " on " + obj.can.id)
	    }
	}
	
	mouseout(event) { this.drag = false }
	
	resize(event)
	{
        console.log("resize body")
	    if (this.resize_processing == 0)
	    {
	        this.resize_processing++
	        this.can.width = this.can.parentElement.offsetWidth
	        this.draw()
	        this.resize_processing -= 1
	    }
	}
	
	handleScroll(event, obj)
	{
	    event.preventDefault()
	    var delta = event.wheelDelta ? event.wheelDelta/40 : event.detail ? -event.detail : 0;
	    var x = event.offsetX || (event.pageX - obj.can.offsetLeft)
	    var y = event.offsetY || (event.pageY - obj.can.offsetTop)
	    if (delta != 0)
	    {
	        var nsf = delta < 0 ? obj.sf / 1.3 : obj.sf * 1.3
	        obj.dx = x - nsf * (x - obj.dx) / obj.sf
	        obj.dy = y - nsf * (y - obj.dy) / obj.sf
	        obj.sf = nsf
	        obj.draw()
	    }
	}
	
	handleTouch(event, obj)
	{
		event.preventDefault()
	    if (event.touches == undefined || event.touches.length == 0 || event.touches.length > 2)
	    	obj.nr_touches = 0
	    else
	    {
	    	var x = 0
	    	var y = 0
	    	var dist = 0
	    	if (event.touches.length == 1)
	    	{
			    x = event.touches[0].pageX
			    y = event.touches[0].pageY
			}
			else
			{
				var x1 = event.touches[0].pageX
				var y1 = event.touches[0].pageY
				var x2 = event.touches[1].pageX
				var y2 = event.touches[1].pageY
			    x = (x1 + x2) / 2
			    y = (y1 + y2) / 2
			    dist = Math.sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2))
			}
			x -= obj.can.offsetLeft
		    y -= obj.can.offsetTop
		    if (obj.nr_touches == event.touches.length)
		    {
		    	var do_draw = false
		        var d_x = x - obj.last_x
		        var d_y = y - obj.last_y
		        if (d_x != 0 || d_y != 0)
		        {
		            obj.dx += d_x
		            obj.dy += d_y
		            do_draw = true
		        }
			    if (dist > 0 && obj.last_dist > 0)
			    {
			        var nsf = obj.sf * dist / obj.last_dist
			        obj.dx = x - nsf * (x - obj.dx) / obj.sf
			        obj.dy = y - nsf * (y - obj.dy) / obj.sf
			        obj.sf = nsf
			        do_draw = true
				}
				if (do_draw)
					obj.draw()
			}
		    obj.last_x = x
		    obj.last_y = y
		    obj.last_dist = dist
		    obj.nr_touches = event.touches.length
		}
	}

	locate_process(proc_nr)
	{
		if (this.busy)
			return
		this.busy = true

		var process = data.processes[proc_nr - 1]
		if (process.y != null)
		{
			this.dx = this.can.width / 2 - process.x * this.sf
			this.dy = this.can.height / 2 - process.y * this.sf
			this.draw()
			this.ctx.fillStyle = "yellow"
			this.ctx.beginPath()
			//console.log("rect "+((can.width - process.iw - elf_length / 2 - 2) * sf) / 2)+", "+((can.height - 12 * sf) / 2 - 10)+", "+(file.w * sf)+", "+(12 * sf))
			this.ctx.rect(this.can.width / 2 - (process.iw + this.elf_length / 2 + 2) * this.sf, (this.can.height - 12 * this.sf) / 2 - 10, (process.iw + this.elf_length + process.ow + 4) * this.sf, 24 * this.sf)
			this.ctx.fill()
			this.ctx.fillStyle = "black"
			this.ctx.strokeStyle = "black"
			this.drawT(process)
		}
		this.show_process_details(process)

		this.busy = false
	}

	link_to_process(proc_nr)
	{
		return "<A HREF=\"#\" onclick=\"javascript:"+this.diagram_name+".locate_process("+proc_nr+")\">Process "+proc_nr + "</A>"
	}

	locate_file(file_nr, proc_nr, x, y)
	{
		if (this.busy)
			return
		this.busy = true

		//console.log("locate_file "+file_nr+", "+x+", "+y)
		var file = data.files[file_nr]
		if (x != undefined && y != undefined)
		{
			this.dx = this.can.width / 2 - x * this.sf
			this.dy = this.can.height / 2 - y * this.sf
			this.draw()
			this.ctx.fillStyle = "yellow"
			this.ctx.beginPath()
			//console.log("rect "+((can.width - file.w * sf) / 2)+", "+((can.height - 12 * sf) / 2 - 10)+", "+(file.w * sf)+", "+(12 * sf))
			this.ctx.rect((this.can.width - file.w * this.sf) / 2, (this.can.height - 12 * this.sf) / 2 - 10, file.w * this.sf, 12 * this.sf)
			this.ctx.fill()
			this.ctx.fillStyle = "black"
			this.fillText(file.label, x, y)
		}
		this.show_file_details(file, proc_nr)

		this.busy = false
	}

	link_to_file(file, proc_nr, x, y)
	{
		return "<A HREF=\"#\" onclick=\"javascript:"+this.diagram_name+".locate_file("+file.nr+","+proc_nr+","+x+","+y+")\">"+file.name + "</A>"
	}

	show_file_details(file, proc_nr)
	{
		if (this.details == null)
			return;

		//console.log("show_file_details " + file.nr + " " + proc_nr)
		var description = "<H2>File "+file.name+"</H2><P><UL>"
		for (var i = 0; i < file.actions.length; i++)
		{
			var action = file.actions[i]
			if (action.proc >= proc_nr)
			{
				if (action.kind == 'W')
				{
					if (action.proc > proc_nr)
						break
					description += "<LI>Produced by " + this.link_to_process(action.proc)
				}
				else if (action.kind == 'R')
					description += "<LI>Input for " + this.link_to_process(action.proc)
				else if (action.kind == 'e')
					description += "<LI>Executed by " + this.link_to_process(action.proc)
				else if (action.kind == 'r')
				{
					description += "<LI>Deleted by " + this.link_to_process(action.proc)
					break
				}
			}
		}
		description += "</UL>"
		if (file.src != undefined)
			description += "Live-bootstrap source file is '" + file.src + "'<BR>"
		if (file.url != undefined)
			description += "URL: <A HREF=\"" + file.url + "\">" + file.url + "</A>"
		
		if (file.lines != undefined && file.lines.length > 0)
		{
			description += "<P><PRE>\n"
			for (var i = 0; i < file.lines.length; i++)
				description += file.lines[i] + "\n"
			description += "</PRE>"
		}
		
		this.details.innerHTML = description
	}

	show_process_details(process)
	{
		if (this.details == null)
			return;

		var description = "<H2>Process "+process.nr+"</H2><P>"
		if (process.parent != null)
			description += "(Executed by " + this.link_to_process(process.parent) + ")"
		description += "<P><UL>"
		for (var i = 0; i < process.actions.length; i++)
		{
			var action = process.actions[i]
			var file = this.data.files[action.file]
			if (action.kind == 'R')
			{
				var x = process.x
				var y = process.y
				for (var i_i = 0; i_i < process.ins.length; i_i++)
				{
					var in_action = process.ins[i_i]
					if (in_action.file == file)
					{
						if (in_action.x != null)
						{
							x = in_action.x
							break
						}
						if (in_action.out != null)
						{
							x = in_action.out.x
							y = in_action.out.y
							break
						}
					}
				}
				description += "<LI>Uses as input " + this.link_to_file(file, process.nr, x, y)
			}
			else if (action.kind == 'e')
			{
				var proc_nr = 0
				for (var i_a = 0; i_a < file.actions.length; i_a++)
				{
					var action = file.actions[i_a]
					if (action.proc == proc_nr)
						break
					else if (action.kind == 'W')
						proc_nr = action.proc
				}
				if (process.elf != null && process.elf.out != null)
					description += "<LI>Executes " + this.link_to_file(file, proc_nr, process.elf.out.x, process.elf.out.y)
			}
			else if (action.kind == 'W')
			{
				var x = process.x
				var y = process.y
				for (var i_i = 0; i_i < process.outs.length; i_i++)
				{
					var out = process.outs[i_i]
					if (out.file == file)
					{
						x = out.x
						y = out.y
					}
				}
				description += "<LI>Produces " + this.link_to_file(file, process.nr, x, y) + "<UL>"
				//if (file.nr == 666) console.log("Process " + process.nr)
				for (var i_a = 0; i_a < file.actions.length; i_a++)
				{
					var action = file.actions[i_a]
					if (action.proc > process.nr)
					{
						if (action.kind == 'W')
							break
						if (action.kind == 'e')
							description += "<LI>Used as executable " + this.link_to_process(action.proc)
						else if (action.kind == 'R')
							description += "<LI>Used as input for " + this.link_to_process(action.proc)
						else if (action.kind == 'r')
						{
							description += "<LI>Deleted by " + this.link_to_process(action.proc)
							break
						}
					}
				}
				
				description += "</UL>"
			}
			else if (action.kind == 'E')
			{
				description += "<LI>Executes " + this.link_to_process(action.child)
			}
		}
		this.details.innerHTML = "</UL>" + description
	}

	
	constructor(_can, _data, _show_loops)
	{
		this.can = _can
		this.ctx = _can.getContext("2d")
		this.can.width = this.can.parentElement.offsetWidth
		this.data = _data
		this.show_loops = _show_loops
		
		this.sf = 5
		this.dx = 600
		this.dy = 420
		
		this.nr_touches = 0
		this.last_x = 0
		this.last_y = 0
		this.last_dist = 0
		
		this.resize_processing = 0

		this.details = null
		this.diagram_name = ""
		this.busy = false
		
		for (var i = 0; i < this.data.files.length; i++)
	    	this.data.files[i].label = this.file_name(this.data.files[i].name)
	    
		this.n = this.data.processes.length
		for (var i = 0; i < this.n; i++)
		{
		    var process = this.data.processes[i]
		    var actions = process.actions
		    var nr_outputs = 0
		    var in_exts = []
		    var out_exts = []
		    for (var a_i = 0; a_i < actions.length; a_i++)
		    {
		        var action = actions[a_i]
		        if (action.kind == 'R')
		        {
		            process.ins.push(this.file_ref(action, action.by, null))
		            var ext = this.file_ext(this.data.files[action.file])
		            if (ext != "" && !in_exts.includes(ext))
		                in_exts.push(ext)
		        }
		        else if (action.kind == 'e')
		            process.elf = this.file_ref(action, action.by, null)
		        else if (actions[a_i].kind == 'W')
		        {
		            process.outs.push(this.file_ref(action, undefined, process))
		            var ext = this.file_ext(this.data.files[action.file])
		            if (ext != "" && !out_exts.includes(ext))
		                out_exts.push(ext)
		        }
		    }
		    if (in_exts.length > 0)
		    {
		        process.ie = in_exts.join(",")
		        process.iw = this.ctx.measureText(process.ie).width + 2
		    }
		    if (process.outs.length > 0)
		    {
		        if (out_exts.length == 0)
		            process.oe = "elf"
		        else if (out_exts.length > 2)
		            process.oe = "*"
		        else
		            process.oe = out_exts.join(",")
		        process.ow = this.ctx.measureText(process.oe).width + 2
		    }
		    //else if (outputs.length > 1)
		    //    console.log("Process "+process.nr+" has "+outputs.length+" outputs: " + outputs.join(" "))
		}
		
		var x = 0
		var y = 0
		this.elf_length = this.ctx.measureText("elf").width
		var seed_y = 60
		for (var i = 0; i < this.n; i++)
		{
		    var process = this.data.processes[i]
		    var executes_processes = this.has_child_processes(process)
		    if (this.hide_process(process))
		        continue
		    
		    if (process.parent != null)
		    {
		        var parent = this.data.processes[process.parent - 1]
		        var top_y = parent.y - parent.h - 20
		        if (y > top_y)
		            y = top_y
		    }
		    process.x = x
		    process.y = y
		    //if (process.elf != null && process.elf.file != undefined && process.elf.y == 0)
		    //{
		    //    console.log(process.elf.file)
		    //}
		    //var show = process.nr == 3
		    //var start_y = y
		    //if (show) console.log("y = " + y)
		    if (process.elf != null && process.elf.out == null && process.elf.file.y == 0)
		    {
		        process.elf.file.y = seed_y
		        process.elf.file.x = x
		        process.elf.file.w = this.ctx.measureText(process.elf.label).width
		        seed_y -= 20
		    }
		    x += this.elf_length / 2 + process.ow + 12
		    var x_text = x
		    if (process.outs.length == 0)
		    {
		        // - process has no output
		        x += 20
		        y -= 30
		    }
		    else if (process.outs.length > 1)
		    {
		        var last_x_center = 0
		        for (var o_i = 0; o_i < process.outs.length; o_i++)
		        {
		            var out = process.outs[o_i]
		            var file = out.file
		            var a_i = file.actions.length - 1
		            for (; a_i >= 0; a_i -= 1)
		            {
		                var action = file.actions[a_i]
		                if (action.kind == 'W' && action.proc == process.nr)
		                    break
		            }
		            var is_used = false
		            for (a_i++; a_i < file.actions.length; a_i++)
		            {
		                var action = file.actions[a_i]
		                if (action.kind == 'W')
		                    break
		                if (action.proc != process.nr && (action.kind == 'R' || action.kind == 'e') && !this.hide_process(this.data.processes[action.proc - 1]))
		                {
		                    is_used = true
		                    break
		                }
		            }
		            if (is_used)
		            {
		                var out_name = this.file_name(out.name)
		                var out_name_w = this.ctx.measureText(out_name).width
		                x_text = x + out_name_w + 1
		                x += out_name_w / 2
		                last_x_center = x
		                out.x = x
		                out.y = y
		                x += out_name_w / 2 + 12
		            }
		        }
		        y -= 30
		        for (var i2 = i + 1; i2 < this.n; i2++)
		            if (!this.hide_process(this.data.processes[i2]))
		            {
		                x = Math.max(x, last_x_center + this.data.processes[i2].iw + this.elf_length / 2 + 12)
		                break
		            }
		    }
		    else
		    {
		        var out = process.outs[0]
		        //if (show) console.log("has output")
		        var out_name = this.file_name(out.name)
		        var out_name_w = this.ctx.measureText(out_name).width
		        x_text = x + out_name_w + 1
		        x += out_name_w / 2
		        out.x = x
		        out.y = y
		        var next_process = null
		        for (var i2 = i + 1; i2 < this.n; i2++)
		            if (!this.hide_process(this.data.processes[i2]))
		            {
		                next_process = this.data.processes[i2]
		                break
		            }
		        var extra_closed = 0
		        if (next_process != null && process.parent != undefined && next_process.parent != process.parent && next_process.parent != process.nr)
		        {
		            for (var parent_proc = process.parent; parent_proc != null; parent_proc = this.data.processes[parent_proc - 1].parent)
		            {
		                if (parent_proc == next_process.parent)
		                    break
		                extra_closed += 10
		            }
		        }
		        //if (show) console.log("Next process "+next_process.nr+" ins "+next_process.ins.length+" exe_proc "+executes_processes)
		        //if (show && next_process.ins.length == 1) console.log(next_process.ins[0].out)
		        if (next_process != null && next_process.ins.length == 1 && next_process.ins[0].out != null && next_process.ins[0].out.proc == process && !executes_processes)
		        {
		            //if (show) console.log("A")
		            // - the output is the only input for the next process
		            x += out_name_w / 2 + 12 + next_process.iw + this.elf_length / 2 + extra_closed
		        }
		        else if (next_process != null && next_process.elf.out != null && next_process.elf.out.proc == process && !this.used_as_input(out.file))
		        {
		            //if (show) console.log("B")
		            y -= 30
		        }
		        else
		        {
		            //if (show) console.log("C")
		            y -= 30
		            x += Math.max(out_name_w / 2 + extra_closed, next_process != null ? next_process.iw + this.elf_length / 2 : 0) + 12
		        }
		    }
		    if (executes_processes)
		        y -= 10
		    //if (show) console.log("y = " + y + " diff " + (this.start_y - y))
		    var extra = 0
		    for (var parent_proc = process.parent; parent_proc != null; parent_proc = parent_proc.parent)
		    {
		        //console.log("parent " + parent_proc)
		        parent_proc = this.data.processes[parent_proc - 1]
		        var prev_w = parent_proc.w
		        parent_proc.w = x_text + extra - parent_proc.x + parent_proc.iw + this.elf_length / 2 + 8
		        if (prev_w >= 0 && parent_proc.w < 0)
		        {
		            //console.log(process.nr + " " + parent_proc.w + " = " + x_text + " + " + extra + " - " + parent_proc.x + " + " + parent_proc.iw + " + " + this.elf_length + " / 2 + 2")
		            //console.log(parent_proc)
		        }
		        parent_proc.h = parent_proc.y - y + extra - 20
		        extra += 10
		    }
		    //console.log(x + "," + y)
		    
		    var in_text_x = process.x - process.iw - this.elf_length / 2
		    for (var i_i = 0; i_i < process.ins.length; i_i++)
		    {
		        var in_file = process.ins[i_i]
		        if (in_file.out != null && in_file.out.x < in_text_x)
		            in_text_x = in_file.out.x
		    }
		    for (var i_i = process.ins.length - 1; i_i >= 0; i_i -= 1)
		    {
		        var in_file = process.ins[i_i]
		        if (in_file.out == null)
		        {
		            var repeated = false
		            for (var i2 = 0; i2 < i_i; i2++)
		                if (in_file.file == process.ins[i2].file)
		                {
		                    repeated = true
		                    break
		                }
		            if (!repeated)
		            {
		                in_text_x -= 20 + in_file.w / 2 + 2
		                in_file.x = in_text_x
		                in_text_x -= in_file.w / 2 + 2
		            }
		        }
		    }
		    //this.ctx.strokeStyle = "black"
		}
		
		this.fn_mousedown = (event) => { this.mousedown(event, this) }
		this.fn_mousemove = (event) => { this.mousemove(event, this) }
		this.fn_mouseup = (event) => { this.mouseup(event, this) }
		this.fn_mouseout = (event) => { this.mouseout(event, this) }
		//this.fn_mouseresize = (event) => { this.resize(event, this) }
		this.fn_handleScroll = function(event) { 
		    event.preventDefault()
		    var delta = event.wheelDelta ? event.wheelDelta/40 : event.detail ? -event.detail : 0;
		    var x = event.offsetX || (event.pageX - this.can.offsetLeft)
		    var y = event.offsetY || (event.pageY - this.can.offsetTop)
		    if (delta != 0)
		    {
		        var nsf = delta < 0 ? this.sf / 1.3 : this.sf * 1.3
		        this.dx = x - nsf * (x - this.dx) / this.sf
		        this.dy = y - nsf * (y - this.dy) / this.sf
		        this.sf = nsf
		        this.draw()
		    }
		}
		this.fn_handleScroll = (event) => { this.handleScroll(event, this) }
		this.fn_handleTouch = (event) => { this.handleTouch(event, this) }
		
		this.can.addEventListener('mousedown',this.fn_mousedown,false)
		this.can.addEventListener('mousemove',this.fn_mousemove,false)
		this.can.addEventListener('mouseup',this.fn_mouseup,false)
		this.can.addEventListener('mouseout',this.fn_mouseout,false);
		
		this.can.addEventListener('DOMMouseScroll', this.fn_handleScroll, false)
		this.can.addEventListener('mousewheel', this.fn_handleScroll, false)
		
		this.can.addEventListener("touchstart", this.fn_handleTouch, false)
		this.can.addEventListener("touchmove", this.fn_handleTouch, false)
		this.can.addEventListener("touchend", this.fn_handleTouch, false)
		this.can.addEventListener("touchcancel", this.fn_handleTouch, false)
		
		this.draw()
	}
};
