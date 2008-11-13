
event_handler $ontimeupdate [
    parameters minsleft
    if (= $minsleft 1) [
        interval (secs 1) [
            local countdown (secsleft)
            if (= $countdown 30) [msg (gameplay "time remaining: 30 seconds")]
            if (= $countdown 15) [msg (gameplay "time remaining: 15 seconds")]
            if (= $countdown 10) [msg (gameplay "time remaining: 10 seconds")]
            if (= $countdown 0) stop
        ]
        cancel_handler
    ]
]
