
function HtmlTable(){

    var table = document.createElement("table");
    table.cellSpacing = 0;
    table.cellPadding = 2;
    var columns = [];
    
    this.columns = function(fields){
        
        columns = fields;
        
        var tr = document.createElement("tr");
        for(var i = 0; i < columns.length; i++){
            var th = document.createElement("th");
            th.appendChild(document.createTextNode(columns[i].label));
            tr.appendChild(th);
        }
        table.appendChild(tr);
    }
    
    this.row = function(data, className){
        var row_access = {};
        var tr = document.createElement("tr");
        tr.className = className;
        for(var i = 0; i < columns.length; i++){
            var td = document.createElement("td");
            var key = columns[i].key;
            td.className = columns[i].className;
            if(key){
                td.appendChild(document.createTextNode(data[key]));
                tr.appendChild(td);
                row_access[key] = td;
            }else{
                var cellFunction = columns[i].cellFunction;
                if(cellFunction){
                    td.appendChild(cellFunction(data));
                    tr.appendChild(td);
                }
            }
        }
        table.appendChild(tr);
        return row_access;
    }
    
    this.attachTo = function(parent){
        parent.appendChild(table);
    }
}
