const Welcome = () => {
    return (
        <div class="flex-grow mx-auto w-full h-full bg-white px-5 md:px-60 py-5 text-xl">
            <div class="text-5xl text-yellow-500 py-5 font-bold">Welcome to Jarvis Winder!</div>
            <p>
                We introduced huge changes! Your Winder is now controlled via this new web interface.
                Our goal is to create a central place that interconnects all of our Jarvis Hardware allowing a unified experience across the whole ecosystem.
            </p>
            <br></br>
            <div class="flex space-x-4 md:flex-nowrap flex-wrap">
                <img class="w-full md:w-1/2 max-h-80" src="/assets/winder.svg" />
                <div class="w-full md:w-1/2 my-2">
                    <b class="text-yellow-500">Changelog:</b>
                    <ul class="list-disc ml-8">
                        <li>Control your Winder offline</li>
                        <li>Live statistics of the winding process</li>
                        <li>Speeds down to 0,5m/s</li>
                        <li>No need to configure spool tension</li>
                        <li>Change Access Point name</li>
                        <li>Smoother ferrari movement</li>
                        <li>Standby mode</li>
                        <li>Unwinding of filament</li>
                        <li>Motor speed graphs</li>
                        <li>And much more...</li>
                    </ul>
                </div>
            </div>
            <div class="bottom-0 flex space-x-4 my-5">
                <a class="underline" href="https://uploads-ssl.webflow.com/5e4d3f0eab4c8c5b404b8ce5/6148de6b6fed0a3ba5772119_Datenblatt-JARVIS%20Winder.pdf">Winder Datasheet PDF</a>
                <br></br>
                <a class="underline" href="https://youtu.be/AUzj1Oo8TtE">Web-Interface Video Tutorial</a>
            </div>
            <div class="flex space-x-2 right-0 bottom-0 float-right">
                <p>A product by</p>
                <img src="/assets/logo.svg" class="h-8" />
            </div>
        </div>
    )
}

export default Welcome
