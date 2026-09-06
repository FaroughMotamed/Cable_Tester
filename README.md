This project is made to design and build a precision cable tester device that can quickly measure resistance/continuity/short of long M12 cables or any other cable with custom length, resistivity and pin number up to 24 pins.



init steps 
            --> init succeeded --> display init succeeded
            --> if failed --> display init failed

check cable presense 
            --> not present --> display insert cable
            --> present     --> display cable detected, make a beep to indicate cable detected

start measuring all cable pins
            --> each pin to all the other pins
            --> switch the ads1220 for each measurrment
            --> calulcate resistance, open, short, crossed
            --> make a final matrix of the results

alayze the results to find out about cable status
            --> see if there is short, open, or normal resistance

display the results on the screen
            --> display detected, short, crossed, resistance ....





