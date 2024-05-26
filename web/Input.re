[@react.component]
let make = (~value, ~onChange: string => unit) => {
  <div>
    <CodeMirror
      value
      onChange={(value, _) => onChange(value)}
      basicSetup={"lineNumbers": false, "foldGutter": false}
    />
  </div>;
};
