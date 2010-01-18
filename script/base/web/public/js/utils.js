
function HtmlTable(){

    var table = document.createElement("table");
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
    
    this.row = function(data){
        var row_access = {};
        var tr = document.createElement("tr");
        for(var i = 0; i < columns.length; i++){
            var td = document.createElement("td");
            td.appendChild(document.createTextNode(data[columns[i].key]));
            tr.appendChild(td);
            row_access[columns[i].key] = td;
        }
        table.appendChild(tr);
        return row_access;
    }
    
    this.attachTo = function(parent){
        parent.appendChild(table);
    }
}
