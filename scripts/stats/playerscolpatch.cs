

try statsdb eval [select players from matches where 0 = 1] [] [
    log_status "Applying stats db update patch..."
    statsdb eval [alter table matches add players integer;] []
    local max 0
    statsdb eval [select max(id) from matches] [max = (column "max(id)")]
    log_status [Need to update @max records] 
    loop i $max [
        local match_id $i
        statsdb eval [update matches set players = (select count(*) from players where match_id = $match_id) where id = $match_id] []
    ]
]
