0different%20engine"/>
<message type="outgoing_privateMessage" time="1138900839" medium="MSN" to="cadetbagpipe%40sccscap%2Ecom" from="thegpfury%40gmail%2Ecom" from_display="thegpfury%40gmail%2Ecom" text="%3AD"/>
<message type="incoming_privateMessage" time="1138900857" medium="MSN" to="thegpfury%40gmail%2Ecom" from="cadetbagpipe%40sccscap%2Ecom" from_display="cadetbagpipe%40sccscap%2Ecom" text="I%20wouldn%27t%20go%20the%20300%20more%20references%20route%2E%2E%2E%2E"/>
<message type="incoming_privateMessage" time="1138900862" medium="MSN" to="thegpfury%40gmail%2Ecom" from="cadetbagpipe%40sccscap%2Ecom" from_display="cadetbagpipe%40sccscap%2Ecom" text="that%20sounds%20like%20a%20lot%20of%20work"/>
<message type="outgoing_privateMessage" time="1138900868" medium="MSN" to="cadetbagpipe%40sccscap%2Ecom" from="thegpfury%40gmail%2Ecom" from_display="thegpfury%40gmail%2Ecom" text="yeah"/>
<message type="outgoing_privateMessage" time="1138900883" medium="MSN" to="cadetbagpipe%40sccscap%2Ecom" from="thegpfury%40gmail%2Ecom" from_display="thegpfury%40gmail%2Ecom" text="it%20might%20be%20less"/>
<message type="outgoing_privateMessage" time="1138900885" medium="MSN" to="cadetbagpipe%40sccscap%2Ecom" from="thegpfury%40gmail%2Ecom" from_display="thegpfury%40gmail%2Ecom" text="I%20am%20guessing"/>
<message type="outgoing_privateMessage" time="1138900886" medium="MSN" to="cadetbagpipe%40sccscap%2Ecom" from="thegpfury%40gmail%2Ecom" from_display="thegpfury%40gmail%2Ecom" text="%3AD"/>
                                    .."  ",",")
  i = i+1
 end
 print(ident.." }")
 print(ident.."}"..close)
end

-- Internal constructor
function _Operator (t)
 t._base = classOperator
 settag(t,tolua_tag)

 if t.const ~= 'const' and t.const ~= '' then
  error("#invalid 'const' specification")
 end

 append(t)
 if not t:inclass() then
  error("#operator can only be defined as class member")
 end

 t.cname = t:cfuncname("toluaI")..t:overload(t)
 t.name = t.name..t.kind
 return t
end

-- Constructor
-- Expects three strings: one representing the function declaration,
-- another representing the argument list, and the third representing
-- the "const" or empty string.
function Operator (d,k,a,c)
 local t = split(strsub(a,2,strlen(a)-1),',') -- eliminate braces
 local i=1
 local l = {n=0}
 while t[i] do
  l.n = l.n+1
  l[l.n] = Declaration(t[i],'var')
  i = i+1
 end
 if k == '[]' then
  d = gsub(d,'&','')
 elseif k=='&[]' then
  l.n = l.n+1
  l[l.n] = Declaration(d,'var')
  l[l.n].name = 'toluaI_value'
 end
 local f = Declaration(d,'func')
 if k == '[]' and (l[1]==nil or isbasic(l[1].type)~='number') then
  error('operator[] can only be defined for numeric index.')
 end
 f.args = l
 f.const = c
 f.kind = gsub(k,"%s","")
 f.lname = _TM[f.kind]
 if not f.lname then
  error("tolua: no support for operator" .. f.kind)
 end
 if f.kind == '[]' and not strfind(f.mod,'const') then
  Operator(d,'&'..k,a,c) 	-- create correspoding set operator
 end
 return _Operator(f)
end


