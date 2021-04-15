--PMP dissector

local p_pmp = Proto("PMP", "Peer Manage Protocol")

local f_type = ProtoField.uint8("PMP.type", "Type", base.DEC)
local f_code = ProtoField.uint8("PMP.code", "Code", base.DEC)
local f_port = ProtoField.uint16("PMP.port", "Port", base.DEC)
local f_tid  = ProtoField.string("PMP.targetID", "Target ID")
local f_sid  = ProtoField.string("PMP.sourceID", "Source ID")
local f_rev  = ProtoField.uint8("PMP.reserve", "Reserve", base.DEC)
local f_count = ProtoField.uint16("PMP.count", "Count", base.DEC)


p_pmp.fields = { f_type, f_code, f_port, f_tid, f_sid, f_rev, f_count}

local data_dis = Dissector.get("data")

function get_type_string(type)
      if type == 0 then
            return "Discovery"
      elseif type == 1 then
            return "Ping"
      elseif type == 2 then
            return "Get Peers"
      else 
            return "Error"
      end
end

function get_code_string(code)
      if code == 0 then
            return "Request"
      else 
            return "Response"
      end
end


function int_to_ip(n)
	if n then
		n = tonumber(n)
		local n1 = math.floor(n / (2^24)) 
		local n2 = math.floor((n - n1*(2^24)) / (2^16))
		local n3 = math.floor((n - n1*(2^24) - n2*(2^16)) / (2^8))
		local n4 = math.floor((n - n1*(2^24) - n2*(2^16) - n3*(2^8)))
    	      return n1.."."..n2.."."..n3.."."..n4 
	end
	return "0.0.0.0"
end


function p_pmp_dissector(buf, pkt, tree)
      local buf_len = buf:len()

      if buf_len < 44 then return false end

      local t = tree:add(p_pmp, buf())

      local v_type = buf(0, 1):uint()
      local v_code = buf(1, 1):uint()
      t:add(buf(0, 1), "Type:".. v_type.." ("..get_type_string(v_type)..")")
      t:add(buf(1, 1), "Code:"..v_code.." ("..get_code_string(v_code)..")")
      --Discovery
      if v_type == 0 then 
            if v_code == 0 then
                  local v_port = buf(2, 2):uint()
                  local v_tid  = buf(4, 20):string()
                  local v_sid  = buf(24, 20):string() 
                  t:add(buf(2, 2), "Port: "..v_port)
                  t:add(buf(4, 20), "Target ID: "..v_tid)
                  t:add(buf(24, 20), "Source ID: "..v_sid)
            else 
                  local v_count = buf(2, 2):uint()
                  local v_tid  = buf(4, 20):string()
                  t:add(buf(2, 2), "Count: "..v_count)
                  t:add(buf(4, 20), "Target ID:"..v_tid)

                  if buf_len < 24 + v_count * 28 then return false end
                  local index = 24
                  local tb = t:add(buf(24, buf_len - 24),"Discovery Options (".. v_count.." results)")
                  for i = 1, v_count do
                        local port = buf(index, 2):uint()
                        local rev  = buf(index + 2, 2):uint()
                        local ipv4 = int_to_ip(buf(index + 4, 4):uint())
                        local id   = buf(index + 8, 20):string()
                        local sub_tb = tb:add(buf(index, 28), i.. "st result "..ipv4..":"..port.." node id: "..id)
                        
                        sub_tb:add(buf(index , 2), "Port: "..port)
                        sub_tb:add(buf(index + 2, 2), "Reserve: "..rev)
                        sub_tb:add(buf(index + 4, 4), "IP: "..ipv4)
                        sub_tb:add(buf(index + 8, 20), "ID: "..id)
                        index = index + 28
                  end
            end
      elseif v_type == 1 then
      
            local v_rev = buf(2, 2):uint()
            local v_tid  = buf(4, 20):string()
            local v_sid  = buf(24, 20):string() 
            t:add(buf(2, 2), "Reserve: "..v_rev)
            t:add(buf(4, 20), "Target ID: "..v_tid)
            t:add(buf(24, 20), "Source ID: "..v_sid)
            
      elseif v_type == 1 then
            if v_code == 0 then
                  local v_rev = buf(2, 2):uint()
                  local v_tid  = buf(4, 20):string()
                  local v_sid  = buf(24, 20):string() 
                  t:add(buf(2, 2), "Reserve: "..v_rev)
                  t:add(buf(4, 20), "Target ID: "..v_tid)
                  t:add(buf(24, 20), "Source ID: "..v_sid)
            end
      end
      return true
end

function p_pmp.dissector(buf,pkt,root) 
      if p_pmp_dissector(buf,pkt,root) then
          --valid ScoreBoard diagram
      else
          --data这个dissector几乎是必不可少的；当发现不是我的协议时，就应该调用data
          data_dis:call(buf,pkt,root)
      end
  end

local udp_encap_table = DissectorTable.get("udp.port")

udp_encap_table:add(9998, p_pmp)
