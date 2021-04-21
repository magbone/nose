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

local type_string = {
      "Discovery",
      "Ping",
      "Get Peers",
      "Registry Peer",
      "Find Peer"
}

local code_string = {
      "Request",
      "Response"
}

local nat_type_string = {
      "UDP Blocked",
      "Sym. UDP Firewall",
      "Open Internet",
      "Full Cone",
      "Restricted",
      "Port Restricted",
      "Symmetric NAT",
      "Unexpected NAT Type"
}

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
      t:add(buf(0, 1), "Type:".. v_type.." ("..type_string[v_type + 1]..")")
      t:add(buf(1, 1), "Code:"..v_code.." ("..code_string[v_code + 1]..")")
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
      -- Ping
      elseif v_type == 1 then
      
            local v_rev = buf(2, 2):uint()
            local v_tid  = buf(4, 20):string()
            local v_sid  = buf(24, 20):string() 
            t:add(buf(2, 2), "Reserve: "..v_rev)
            t:add(buf(4, 20), "Target ID: "..v_tid)
            t:add(buf(24, 20), "Source ID: "..v_sid)
      -- Get Peers      
      elseif v_type == 2 then
            if v_code == 0 then
                  local v_rev = buf(2, 2):uint()
                  local v_tid  = buf(4, 20):string()
                  local v_sid  = buf(24, 20):string() 
                  t:add(buf(2, 2), "Reserve: "..v_rev)
                  t:add(buf(4, 20), "Target ID: "..v_tid)
                  t:add(buf(24, 20), "Source ID: "..v_sid)
            else
                  local v_count = buf(2, 2):uint()
                  local v_tid  = buf(4, 20):string()
                  t:add(buf(2, 2), "Count: "..v_count)
                  t:add(buf(4, 20), "Target ID:"..v_tid)

                  if buf_len < 24 + v_count * 32 then return false end
                  local index = 24
                  local tb = t:add(buf(24, buf_len - 24),"Get Peers Options (".. v_count.." results)")
                  for i = 1, v_count do
                        local nat_type    = buf(index, 1):uint()
                        local rev         = buf(index + 1, 1):uint()
                        local port        = buf(index + 2, 2):uint()
                        local ipv4        = int_to_ip(buf(index + 4, 4):uint())
                        local vlan_ipv4   = int_to_ip(buf(index + 8, 4):uint())
                        local id          = buf(index + 12, 20):string()
                        local sub_tb      = tb:add(buf(index, 28), i.. "st result "..ipv4..":"..port.." nat type: "
                              ..nat_type_string[nat_type + 1].." node id: "..id.." Vlan IP: "..vlan_ipv4)
                        
                        sub_tb:add(buf(index , 1), "Nat Type: "..nat_type.." ("..nat_type_string[nat_type + 1]..")")
                        sub_tb:add(buf(index + 1, 1), "Reserve: "..rev)
                        sub_tb:add(buf(index + 2, 2), "Port: "..port)
                        sub_tb:add(buf(index + 4, 4), "IP: "..ipv4)
                        sub_tb:add(buf(index + 8, 4), "VLAN IP:"..vlan_ipv4)
                        sub_tb:add(buf(index + 12, 20), "ID: "..id)
                        index = index + 32
                  end
            end
      -- Registry Peer
      elseif v_type == 3 then
            if v_code == 0 then
                  if buf_len < 56 then return false end
                  local nat_type = buf(2, 2):uint()
                  local reserved = buf(4, 2):uint()
                  local port     = buf(6, 2):uint()
                  local ipv4     = int_to_ip(buf(8, 4):uint())
                  local vlan_ipv4= int_to_ip(buf(12, 4):uint())
                  local mstp_id  = buf(16, 20):string()
                  local peer_id  = buf(36, 20):string()

                  t:add(buf(2, 2), "NAT Type: "..nat_type.." ("..nat_type_string[nat_type + 1]..")")
                  t:add(buf(4, 2), "Reserved: "..reserved)
                  t:add(buf(6, 2), "Port: "..port)
                  t:add(buf(8, 4), "IP: "..ipv4)
                  t:add(buf(12, 4), "VLAN IP: "..vlan_ipv4)
                  t:add(buf(16, 20), "Master Peer ID: ".. mstp_id)
                  t:add(buf(36, 20), "Peer ID: "..peer_id)
            else
                  local v_rev = buf(2, 2):uint()
                  local v_tid  = buf(4, 20):string()
                  local v_sid  = buf(24, 20):string() 
                  t:add(buf(2, 2), "Reserve: "..v_rev)
                  t:add(buf(4, 20), "Peer ID: "..v_tid)
                  t:add(buf(24, 20), "Master Peer ID: "..v_sid)
            end
      elseif v_type == 4 then
            if v_code == 0 then
                  if  buf_len < 48 then return false end

                  local reserve     = buf(2, 2):uint()
                  local vlan_ipv4   = int_to_ip(buf(4, 4):uint())
                  local mstp_id     = buf(8, 20):string()
                  local peer_id     = buf(28, 20):string()

                  t:add(buf(2, 2), "Reserve: ".. reserve)
                  t:add(buf(4, 4), "VLAN IP: "..vlan_ipv4)
                  t:add(buf(8, 20), "Master peer ID: ".. mstp_id)
                  t:add(buf(28, 20), "Peer ID: ".. peer_id)
            else 
                  local nat_type = buf(2, 2):uint()
                  local reserved = buf(4, 2):uint()
                  local port     = buf(6, 2):uint()
                  local ipv4     = int_to_ip(buf(8, 4):uint())
                  local vlan_ipv4= int_to_ip(buf(12, 4):uint())
                  local mstp_id  = buf(16, 20):string()
                  local peer_id  = buf(36, 20):string()

                  t:add(buf(2, 2), "NAT Type: "..nat_type.." ("..nat_type_string[nat_type + 1]..")")
                  t:add(buf(4, 2), "Reserved: "..reserved)
                  t:add(buf(6, 2), "Port: "..port)
                  t:add(buf(8, 4), "IP: "..ipv4)
                  t:add(buf(12, 4), "VLAN IP: "..vlan_ipv4)
                  t:add(buf(16, 20), "Master Peer ID: ".. mstp_id)
                  t:add(buf(36, 20), "Peer ID: "..peer_id)
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
