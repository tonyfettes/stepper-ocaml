module Object = {
  [@react.component]
  let make = (~ctx, ~exp, ~onClick) => {
    switch (ctx |> Stepper.Ctx.to_string(~hide_filters=true) |> String.split_on_char('@')) {
    | [prefix, suffix] =>
      <div className="whitespace-pre font-mono">
        prefix->React.string
        <span
          className={"cursor-pointer text-blue-500"}
          onClick={_ => onClick(ctx, exp)}
        >
          {exp->Syntax.Exp.to_string->React.string}
        </span>
        suffix->React.string
      </div>;
    | _ => failwith("Invalid context")    
    }
  };
};

module Result = {
  [@react.component]
  let make = (~value, ~onClick) => {
    <div>
      {switch (value) {
       | `Err(value) => value->React.string
       | `Val(value) =>
         <div> {value->Syntax.Exp.to_string->React.string} </div>
       | `Exp(value) =>
         value
         ->Belt.List.map(((ctx, exp)) => {<Object ctx exp onClick />})
         ->Belt.List.toArray
         ->React.array
       }}
    </div>;
  };
};

module Input = {
  [@react.component]
  let make = (~value, ~onChange: string => unit) => {
    <input
      className="w-full font-mono focus:outline-none"
      type_="text"
      value
      onChange={e => {
        let source = e->React.Event.Form.target##value;
        onChange(source);
      }}
    />;
  };
};

module History = {
  [@react.component]
  let make = (~history) => {
    <div>
      {history
       ->Belt.List.map(((ctx, exp)) => {
           <Object ctx exp onClick={(_, _) => ()} />
         })
       ->Belt.List.toArray
       ->React.array}
    </div>;
  };
};

module Stepper = {
  // This sample forces an import of Belt.*, so that CI builds can ensure that
  // Melange has been installed correctly for JS bundlers to be able to find it.
  [@react.component]
  let make = () => {
    let (input, setInput) = React.useState(() => "");
    let (history, setHistory) = React.useState(() => []);
    let (result, setResult) = React.useState(() => `Exp([]));

    let updateResult = input => {
      print_endline("source: " ++ input);
      let buf = Lexing.from_string(input);
      switch (Parser.top(Lexer.lex, buf)) {
      | exception (Lexer.Error(message)) =>
        setResult(_ => `Err("Lexing error: " ++ message))
      | exception Parser.Error
      | None => setResult(_ => `Err("Syntax error"))
      | Some(exp) => setResult(_ => Stepper.step(exp))
      };
    };

    let onStep = (ctx, exp) => {
      setHistory(history => [(ctx, exp), ...history]);
      switch (Stepper.transition(exp)) {
      | `Err(_, _) => ()
      | `Exp(exp)
      | `Val(exp) =>
        let exp = Stepper.Ctx.compose(ctx, exp);
        let exp = exp->Syntax.Exp.to_string;
        updateResult(exp);
      };
    };

    <div>
      <Input
        value=input
        onChange={input => {
          setHistory(_ => []);
          setInput(_ => input);
          updateResult(input);
        }}
      />
      <hr />
      <History history={history->List.rev} />
      <hr />
      <Result value=result onClick=onStep />
    </div>;
  };
};

module App = {
  [@react.component]
  let make = () => {
    <div className="p-8"> <Stepper /> </div>;
  };
};

switch (ReactDOM.querySelector("#root")) {
| Some(root) => ReactDOM.Client.(createRoot(root)->render(<App />));
| None =>
  Js.Console.error("Failed to start React: couldn't find the #root element")
};
