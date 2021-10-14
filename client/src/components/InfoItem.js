const InfoItem = ({ data, headline }) => {
    return (
        <div class="mb-2">
            <div class="mb-1 text-center underline">{headline}:</div>
            <div class="flex space-x-4">
                <div class="text-left">
                    {
                        Object.keys(data).map(key => {
                            return <div>{key}</div>
                        })
                    }
                </div>
                <div class="text-left">
                    {
                        Object.values(data).map(value => {
                            return <div>{value}</div>
                        })
                    }
                </div>
            </div>
        </div>
    )
}

export default InfoItem
