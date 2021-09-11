const filamentDiameter = 1.75;
const spoolWidth = 500;
const wallThickness = 30;
const spoolInnerDiameter = 500;
const spoolDiameter = 1500;

const windingsPerLayer = spoolWidth/filamentDiameter;

const Spool = (windings) => {
    const layerCount = Math.floor(windings / windingsPerLayer);
    const layerWidth = spoolInnerDiameter + (layerCount * filamentDiameter * 2);
    const layerHeight = windings % windingsPerLayer;
    const lastLayerWidth = layerCount != 0 ? (layerWidth - 2*filamentDiameter) : 0;

    return (
        <>
        <rect x="0" y="0" width={spoolDiameter} height={wallThickness} style="fill:rgb(0,0,255);stroke-width:3;stroke:rgb(0,0,0)" />
        <rect x="0" y={wallThickness + spoolWidth} width={spoolDiameter} height={wallThickness} style="fill:rgb(0,0,255);stroke-width:3;stroke:rgb(0,0,0)" />
        </>
    )
}

export default Spool
