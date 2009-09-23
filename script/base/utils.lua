
function server.deleteFile(filename)
    local file = io.open(filename, "w+")
    file:close()
    os.remove(filename)
end

function server.fileExists(filename)
    local file = io.open(filename, "r")
    if file ~= nil then return true else return false end
end
