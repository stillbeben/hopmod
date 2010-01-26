
function HtmlTable(){

    var table = document.createElement("table");
    table.cellSpacing = 0;
    table.cellPadding = 2;
    
    var columns = [];
    var sortPriority = [];
    
    var last_row_access;
    
    this.columns = function(fields, sortByFields){
        
        columns = fields;
        
        var tr = document.createElement("tr");
        for(var i = 0; i < columns.length; i++){
            var th = document.createElement("th");
            th.appendChild(document.createTextNode(columns[i].label));
            tr.appendChild(th);
        }
        table.appendChild(tr);
        
        sortPriority = sortByFields || [];
    }
        
    this.row = function(data, className){
        
        var rowInterface = {};
        rowInterface.field = {};
        
        var tr = document.createElement("tr");
        tr.className = className;
        rowInterface.tableRowElement = tr;
        
        for(var i = 0; i < columns.length; i++){
            var td = document.createElement("td");
            var key = columns[i].key;
            td.className = columns[i].className || "";
            if(key){
                td.appendChild(document.createTextNode(data[key]));
                tr.appendChild(td);
                
                rowInterface.field[key] = {
                    update: 
                }
            }else{
                var cellFunction = columns[i].cellFunction;
                if(cellFunction){
                    td.appendChild(cellFunction(data));
                    tr.appendChild(td);
                }
            }
        }
        
        table.appendChild(tr);
        
        return rowInterface;
    }
    
    this.attachTo = function(parent){
        parent.appendChild(table);
    }
}
